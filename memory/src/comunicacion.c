#include <comunicacion.h>

int fd_memoria;
pthread_t crear_server_memoria;
char* ip_memory;
char* puerto_memory;
sem_t semaforoPrueba;

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
                log_info(logger_memory,"Como handshake, CPU me solicito entradas por tabla y tamanio de las paginas");
                recibirOrden(cliente_socket);
                uint32_t * array = calloc(3,sizeof (uint32_t));
                array[0] = 2;
                array[1] = cfg_memory->ENTRADAS_POR_TABLA;
                array[2] = cfg_memory->TAM_PAGINA;
                log_info(logger_memory,"Se envio a CPU lo que solicito, entradas por tabla: <%d> y tamanio de pagina: <%d>", array[1],array[2]);
                enviar_int_array(array,cliente_socket,HANDSHAKE_CPU,logger_memory);
                free(array);
                break;
            }
            case PROCESO_INICIADO:
                proceso_iniciado(cliente_socket);

                break;
            case PROCESO_TERMINADO:
                proceso_terminado(cliente_socket);
                break;
            case PAGE_FAULT:
                page_fault(cliente_socket);
                break;
            case SOLICITUD_MARCO:
                solicitud_marco(cliente_socket);
                break;
            case PEDIDO_ESCRITURA:
                pedido_escritura(cliente_socket);
                break;
            case PEDIDO_LECTURA:
                pedido_lectura(cliente_socket);
                break;
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
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t atenderConexionNueva;
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&atenderConexionNueva, NULL,procesar_conexion,args);
        pthread_detach(atenderConexionNueva);
        return 1;
    }
    return 0;
}

bool generar_conexiones(){

    pthread_create(&crear_server_memoria,NULL, crearServidor,NULL);
    pthread_join(crear_server_memoria, NULL);
    return true; //debe tomarse lo que retorna el hilo al crear el servidor
}

void cerrar_servers(){
    close(fd_memoria);
    log_info(logger_memory,"SERVIDORES CERRADOS");
}

void* crearServidor(){
    puerto_memory=cfg_memory->PUERTO_ESCUCHA;
    fd_memoria = iniciar_servidor(logger_memory, "SERVER MEMORIA", ip_memory,puerto_memory);

    if (fd_memoria == 0) {
        log_error(logger_memory, "Fallo al crear el servidor MEMORIA, cerrando MEMORIA");
        return EXIT_FAILURE;
    }

    while (server_escuchar(logger_memory, "SERVER MEMORIA", fd_memoria));
}

///////////////////////////////////////////////////
////RECEPCION DE MENSAJES////
///////////////////////////////////////////////////



t_process_memory * crearNuevoProcesoEnMemoria(uint32_t pid, uint32_t* indices){
    t_process_memory * nuevoProceso = malloc(sizeof(t_process_memory));
    t_queue* colaPaginasEnMP = queue_create();
    t_list* indicesAsList = convertirIntArrayALista(indices);
    nuevoProceso->indicesTablasPaginas = indicesAsList;
    nuevoProceso->pid=pid;
    nuevoProceso->paginasEnMP = colaPaginasEnMP;
    return nuevoProceso;
}

uint32_t* crearArrayConPID(uint32_t* array, uint32_t pid){
    uint32_t size = array[0]+1;

    uint32_t* nuevoArray=calloc(size + 1, sizeof(uint32_t));
    nuevoArray[0] = size;
    nuevoArray[1] = pid; // [7, 1, elementos]
    for (int i = 0; i <size-1 ; i++) {
        nuevoArray[i+2]= array[i+1];
    }
    return nuevoArray;

}

void proceso_iniciado(int cliente_socket){
    //DEBERIA RECIBIR PID + CANTIDAD SEGMENTOS
    uint32_t *array = recibir_int_array(cliente_socket);
    uint32_t *arrayPosiciones = inicializar_estructuras_proceso(array);
    t_process_memory* nuevoProceso = crearNuevoProcesoEnMemoria(array[1], arrayPosiciones);
    uint32_t* arrayConPID = crearArrayConPID(arrayPosiciones, array[1]);
    list_add(procesos, nuevoProceso);
    enviar_int_array(arrayConPID, cliente_socket, PROCESO_INICIADO,logger_memory);

    free(array);
    //free(arrayPosiciones);
    free(arrayConPID);
}


void proceso_terminado(int cliente_socket){
    uint32_t pid = recibirValor_uint32(cliente_socket,logger_memory);
    liberar_estructuras_proceso(pid);
    enviarOrden(PAGINAS_LIBERADAS, cliente_socket,logger_memory);
}

void page_fault(int cliente_socket){
    log_info(logger_memory,"Espero data del page fault");
    uint32_t *array = recibir_int_array(cliente_socket);
    log_info(logger_memory,"Recibi la data del page fault");
    int comprobacion = cargarEnMP(array[1],array[2],array[3]);
    if(comprobacion == NULL){
        log_error(logger_memory,"NO SE CARGO EN MP");
        return;
    }
    yaLaCargueEnMP(cliente_socket, array[1]);
    free(array);
    log_debug(logger_memory,"Ya cargue en MP");
}


void pedido_lectura(int cliente_socket){

    int *array = recibir_int_array(cliente_socket);
    log_info(logger_memory,"PID: <%d> - Accion: LEER - Direccion fisica: <%d> - que cae en el Marco: <%d>", array[1],array[2],array[3]);
    simularRetardoEspacioUsuario();
    uint32_t valor = leer_memoria_contigua(array[2], array[1], 0);
    log_info(logger_memory,"En la direccion: %d, se almacena el valor: %d", array[2], valor);
    enviarValor_uint32(valor, cliente_socket, LECTURA_REALIZADA,logger_memory);
    log_info(logger_memory, "Se envio el valor leido: <%d> a CPU", valor);
    free(array);
}

void pedido_escritura(int cliente_socket){
    int *array = recibir_int_array(cliente_socket);
    log_info(logger_memory,"PID: <%d> - Accion: ESCRIBIR - Direccion fisica: <%d> - Valor a escribir: <%d>", array[1],array[2], array[3]);
    if(!escribir_memoria_contigua(array[2], array[3], array[4])){
        log_error(logger_memory, "No se pudo escribir en la posicion %d, el valor %d", array[2], array[3]);
        return;
    }
    free(array);
    enviarOrden(ESCRITURA_REALIZADA,cliente_socket,logger_memory );
}


void solicitud_marco(int cliente_socket){
    uint32_t *array = recibir_pid_indice_tabla_y_pagina(cliente_socket);
    simularRetardoTablaPaginas();
    uint32_t marco = buscarMarcoAsociado(array[2],array[3]);

    if(marco== -1){
        log_info(logger_memory,"PID: <%d> - Pagina: <%d> - Marco: <PAGE FAULT>", array[1],array[3]);
        enviarValor_uint32(array[3],cliente_socket,ACCESO_RESULTO_PAGE_FAULT,logger_memory);
        free(array);
        return;
    }else if(marco == -2){
        log_info(logger_memory,"PID: <%d> - Pagina: <%d> - Marco: <???>", array[1],array[3]);
        enviarValor_uint32(array[3],cliente_socket,ERROR_INDICE_TP,logger_memory);
        free(array);
        return;
    }
    log_info(logger_memory,"Se envio a CPU del proceso con PID: <%d> - la Pagina: <%d> - el Marco: <%d>", array[1],array[3],marco);
    enviarValor_uint32(marco,cliente_socket,ENVIO_MARCO_CORRESPONDIENTE,logger_memory);
    free(array);
    //recibir pagina
    //si esta cargado, devolvemos numero de marco
    //si no esta cargado, devolvemos page fault
}


///////////////////////////////////////////////////
////AUXILIARES DE LA RECEPCION DE MENSAJES////
///////////////////////////////////////////////////


int* recibir_pid_indice_tabla_y_pagina(int cliente_socket){
    return recibir_int_array(cliente_socket);
}

int* recibir_indices_tablas_de_paginas(int cliente_socket){
    return recibir_int_array(cliente_socket);
}

int yaLaCargueEnMP(int cliente_socket, uint32_t pid){
    log_info(logger_memory,"Se aviso a Kernel que la pagina correspondiente al PID: <%d> fue cargada", pid);
    enviarValor_uint32(pid, cliente_socket, PROCESO_CARGADO_EN_MP,logger_memory);

    //TODO comprobacion
    return true;
}
