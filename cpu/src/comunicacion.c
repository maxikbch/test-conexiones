#include <comunicacion.h>
#include <utils/test_serializacion.h>

int fd_memoria = -1;
int fd_dispatch = -1;
int fd_interrupt = -1;
int fd_kernel = -1;
char *ip_cpu;
pthread_t crear_server_dispatch;
pthread_t crear_server_interrupt;
pthread_t ciclo_instrucciones_thread;
int cliente_socket; //Siempre es Kernel
pcb *pcb_actual;
pthread_t conexion_memoria;
sem_t semHayPCBCargada;
sem_t semNoEstoyEjecutando;
sem_t semEsperandoRespuestaDeMarco;
sem_t sem_interrupt;
bool inicializacion_fd_completada = false;
//INTERRUPCION
int interrupcion=0;



// Esta funcion todavia no no es util. Sirve para interpretar las cosas que le va mandando la consola por el socket
static void procesar_conexion(void *void_args) {
    t_procesar_conexion_args *args = (t_procesar_conexion_args *) void_args;
    t_log *logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    while (cliente_socket != -1) {

        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }


        switch (cop) {
            case DEBUG:
                log_info(logger, "debug");
                break;
            case HANDSHAKE_CPU:{
                int *array = recibir_int_array(cliente_socket);
                entradasPorTabla = array[1];
                tamanioDePagina = array[2];
                log_info(logger_cpu,"Memoria me envio la cantidad de entradas por tabla: <%d> y el tamanio de pagina: <%d>",array[1],array[2]);
                break;
            }

            case PCB:
            {
                if(!inicializacion_fd_completada){
                    fd_kernel = cliente_socket;
                    inicializacion_fd_completada = true;
                }
                pcb_actual = recibir_pcb(cliente_socket);
                log_info(logger_cpu,"Arribo de PCB: PID: <%d> ", pcb_actual->id);
                //TODO ACA PONES EL PC DESEADO y el OPCODE
                tengoQueEsperarRespuestaContinuar = false;
                ciclo_instrucciones();

                break;
            }

            case INTERRUPCION:{
                //LOGGER SOLICITUD DE INTERRUPCION POR KERNEL
                recibirOrden(cliente_socket);
                log_info(logger_cpu,"Recibi una interrupcion de KERNEL");
                interrupcion = 1;
                sem_wait(&sem_interrupt); //Bloqueo hilo interrupt
                break;
            }
            case ACCESO_RESULTO_PAGE_FAULT:
            {

                uint32_t valor = recibirValor_uint32(cliente_socket, logger_cpu);
                log_info(logger_cpu,"La pagina: <%d> resulto en <PAGE FAULT>", valor);
                page_fault();
                break;
            }
            case ENVIO_MARCO_CORRESPONDIENTE:
            {
                recibir_marco_memoria();
                break;
            }
            case LECTURA_REALIZADA:{
                uint32_t valor = recibirValor_uint32(cliente_socket, logger_cpu);
                log_info(logger_cpu, "Memoria confirma la lectura del valor <%d> ", valor);
                terminar_ejecucion_lectura(valor);
                break;
            }
            case ESCRITURA_REALIZADA:
            {
                recibirOrden(cliente_socket);
                log_info(logger_cpu, "Memoria confirma la escritura del valor solicitado");
                terminar_ejecucion_escritura();
                break;
            }

            case ERROR_INDICE_TP:
            {
                uint32_t valor =recibirValor_uint32(cliente_socket, logger_cpu);
                log_error(logger_cpu,"Se envio un indice de tabla de paginas invalido a memoria: <%d> ",valor);
                return;
            }
            case -1:
                log_error(logger, "Cliente desconectado de %s...", server_name);
                return;

            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
        }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}

int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&atenderProcesoNuevo, NULL, procesar_conexion, args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

void cortar_conexiones() {
    liberar_conexion(&fd_memoria);
    log_info(logger_cpu, "CONEXIONES LIBERADAS");
}

void cerrar_servers() {
    close(fd_dispatch);
    close(fd_interrupt);
    log_info(logger_cpu, "SERVIDORES CERRADOS");
}

void procesoTerminado() {
    //sem_wait(&semHayPCBCargada);
    enviar_paquete_pcb(pcb_actual, cliente_socket, PROCESO_TERMINADO,logger_cpu);
};

void procesoBloqueado();

void procesoDesalojado();

void cpuVacia();


bool generar_conexiones() {
    //TODO cerrar ante un fallo de conexión
    int *estado;

    pthread_create(&conexion_memoria, NULL, conectarConMemoria, NULL);
    activar_cpu();
    //pthread_create(&ciclo_instrucciones_thread, NULL, ciclo_instrucciones, NULL);
    pthread_create(&crear_server_dispatch, NULL, crearServidorDispatch, NULL);
    pthread_create(&crear_server_interrupt, NULL, crearServidorInterrupt, NULL);
    //pthread_join(ciclo_instrucciones_thread, NULL);
    pthread_join(crear_server_dispatch, NULL);
    pthread_join(crear_server_interrupt, NULL);
    pthread_join(conexion_memoria, (void **) &estado);
    return estado;

}


void* conectarConMemoria(){
    int comprobacion = generarConexionesConMemoria();
    atenderMemoria();
}


bool atenderMemoria(){
    if (fd_memoria == -1){
        return EXIT_FAILURE;
    }
    pthread_t atenderMemoria;
    t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
    args->log = logger_cpu;
    args->fd = fd_memoria;
    args->server_name = "ATENDER_MEMORIA";
    pthread_create(&atenderMemoria, NULL,procesar_conexion,args);
    pthread_detach(atenderMemoria);
    return true;
}

bool generarConexionesConMemoria() {
    char *ip;

    ip = strdup(cfg_cpu->IP_MEMORIA);
    log_info(logger_cpu, "Lei la ip %s", ip);

    char *puerto;
    puerto = strdup(cfg_cpu->PUERTO_MEMORIA);

    log_info(logger_cpu, "Lei el puerto %s", puerto);

    fd_memoria = crear_conexion(
            logger_cpu,
            "SERVER MEMORIA",
            ip,
            puerto
    );
    int *estado = malloc(sizeof(int));
    *estado = fd_memoria != 0;

    enviarOrden(HANDSHAKE_CPU, fd_memoria, logger_cpu);
    free(ip);
    free(puerto);

    return (void *) estado;

}


void *crearServidorDispatch() {
    fd_dispatch = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", ip_cpu, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);

    if (fd_dispatch == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor DISPATCH, cerrando CPU");
        return EXIT_FAILURE;
    }

    while (server_escuchar(logger_cpu, "SERVER CPU DISPATCH", (uint32_t) fd_dispatch));
}

void *crearServidorInterrupt() {
    fd_interrupt = iniciar_servidor(logger_cpu, "SERVER CPU INTERRUPT", ip_cpu, cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);

    if (fd_interrupt == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor INTERRUPT, cerrando CPU");
        return EXIT_FAILURE;
    }

    while (server_escuchar(logger_cpu, "SERVER CPU INTERRUPT", (uint32_t) fd_interrupt));
}


void ejecucionPcbTest(pcb* pcb_actual,int cliente_socket,op_code codigoOperacion, uint32_t programCounterDeseado){
    pcb_actual->programCounter = programCounterDeseado;
    enviar_paquete_pcb(pcb_actual,cliente_socket,codigoOperacion,logger_cpu);
}