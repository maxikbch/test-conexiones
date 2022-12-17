#include "../include/init.h"

int checkProperties(char *path) {

    t_config *config = config_create(path);
    if (config == NULL) {
        printf("No se pudo crear la config");
        return false;
    }


    char *properties[] = {
            "IP_MEMORIA",
            "PUERTO_MEMORIA",
            "IP_CPU",
            "PUERTO_CPU_DISPATCH",
            "PUERTO_CPU_INTERRUPT",
            "PUERTO_ESCUCHA",
            "ALGORITMO_PLANIFICACION",
            "GRADO_MAX_MULTIPROGRAMACION",
            "DISPOSITIVOS_IO",
            "TIEMPOS_IO",
            "QUANTUM_RR",
            NULL};

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties)) {
        log_error(logger_kernel, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}

bool cargar_configuracion_perse(t_config* file_cfg_kernel){
    cfg_kernel->IP_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "IP_MEMORIA"));
    cfg_kernel->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_MEMORIA"));
    cfg_kernel->IP_CPU = strdup(config_get_string_value(file_cfg_kernel, "IP_CPU"));
    cfg_kernel->PUERTO_CPU_DISPATCH = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_CPU_DISPATCH"));
    cfg_kernel->PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_CPU_INTERRUPT"));
    cfg_kernel->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_ESCUCHA"));
    cfg_kernel->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(file_cfg_kernel, "ALGORITMO_PLANIFICACION"));
    cfg_kernel->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(file_cfg_kernel, "GRADO_MAX_MULTIPROGRAMACION");
    cfg_kernel->DISPOSITIVOS_IO = config_get_array_value(file_cfg_kernel, "DISPOSITIVOS_IO");
    //log_info(logger_kernel, "DISPOSITIVOS_IO Cargada Correctamente: %s", cfg_kernel->DISPOSITIVOS_IO);
    cfg_kernel->TIEMPOS_IO = config_get_array_value(file_cfg_kernel, "TIEMPOS_IO");
    //log_info(logger_kernel, "TIEMPOS_IO Cargada Correctamente: %s", cfg_kernel->TIEMPOS_IO);
    cfg_kernel->QUANTUM_RR = config_get_int_value(file_cfg_kernel, "QUANTUM_RR");

    return true;
}
bool cargar_configuracion_con_log(t_config* file_cfg_kernel){
    cfg_kernel->IP_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "IP_MEMORIA"));
    log_info(logger_kernel, "IP_MEMORIA Cargada Correctamente: %s", cfg_kernel->IP_MEMORIA);

    cfg_kernel->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_MEMORIA"));
    log_info(logger_kernel, "PUERTO_MEMORIA Cargada Correctamente: %s", cfg_kernel->PUERTO_MEMORIA);

    cfg_kernel->IP_CPU = strdup(config_get_string_value(file_cfg_kernel, "IP_CPU"));
    log_info(logger_kernel, "IP_CPU Cargada Correctamente: %s", cfg_kernel->IP_CPU);

    cfg_kernel->PUERTO_CPU_DISPATCH = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_CPU_DISPATCH"));
    log_info(logger_kernel, "PUERTO_CPU_DISPATCH Cargada Correctamente: %s", cfg_kernel->PUERTO_CPU_DISPATCH);

    cfg_kernel->PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_CPU_INTERRUPT"));
    log_info(logger_kernel, "PUERTO_CPU_INTERRUPT Cargada Correctamente: %s", cfg_kernel->PUERTO_CPU_INTERRUPT);

    cfg_kernel->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_kernel, "PUERTO_ESCUCHA"));
    log_info(logger_kernel, "PUERTO_ESCUCHA Cargada Correctamente: %s", cfg_kernel->PUERTO_ESCUCHA);

    cfg_kernel->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(file_cfg_kernel, "ALGORITMO_PLANIFICACION"));
    log_info(logger_kernel, "ALGORITMO_PLANIFICACION Cargada Correctamente: %s", cfg_kernel->ALGORITMO_PLANIFICACION);

    cfg_kernel->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(file_cfg_kernel, "GRADO_MAX_MULTIPROGRAMACION");
    log_info(logger_kernel, "GRADO_MAX_MULTIPROGRAMACION Cargada Correctamente: %d",
             cfg_kernel->GRADO_MAX_MULTIPROGRAMACION);


    cfg_kernel->DISPOSITIVOS_IO = config_get_array_value(file_cfg_kernel, "DISPOSITIVOS_IO");
    //log_info(logger_kernel, "DISPOSITIVOS_IO Cargada Correctamente: %s", cfg_kernel->DISPOSITIVOS_IO);

    cfg_kernel->TIEMPOS_IO = config_get_array_value(file_cfg_kernel, "TIEMPOS_IO");
    //log_info(logger_kernel, "TIEMPOS_IO Cargada Correctamente: %s", cfg_kernel->TIEMPOS_IO);

    cfg_kernel->QUANTUM_RR = config_get_int_value(file_cfg_kernel, "QUANTUM_RR");
    log_info(logger_kernel, "QUANTUM_RR Cargada Correctamente: %d", cfg_kernel->QUANTUM_RR);

    return true;
}

int cargar_configuracion(char *path) {


    file_cfg_kernel = config_create(path);
    if(mostrarConfig){
        cargar_configuracion_con_log(file_cfg_kernel);
    }else{
        cargar_configuracion_perse(file_cfg_kernel);
    }


    log_info(logger_kernel, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_kernel);

    //--------------------------------------------------------------------------
    int dim = tamanioArray(cfg_kernel->DISPOSITIVOS_IO);
    semaforos_io = calloc(dim,sizeof(sem_t));
    iniciarSemaforoDinamico(semaforos_io, dim);

    cargarDispositivosIo();


    return true;
}

int tamanioArray(char ** array){
    int n=0;
    for(int i=0 ;*(array+i)!= NULL; i++)
        n++;
    return n;
}
t_list* nueva;

void* atenderColaDispositivo(void* void_args){
    t_atenderColaDispositivo_args *args = (t_atenderColaDispositivo_args *) void_args;
    int indice = args->indiceSemaforo;
    int tiempoRetardo = args->tiempoRetardo;
    t_queue* cola= &args->cola;
    char* nombre = args->nombre;

    while(1){
        //TODO Hacer un sem_post cada vez que se agrega un proceso a la cola

        //log_info(logger_kernel,"Esperando a entrar al hilo de IO");

        sem_wait(&semaforos_io[indice]);

     //if(!queue_is_empty(cola)){

         pcb* pcbRecibida = queue_pop(cola);
         uint32_t pcActual= pcbRecibida->programCounter -1;
         instr_t* instruccionActual = list_get(pcbRecibida->instr,pcActual);
         int duracion = atoi(instruccionActual->param2);
        log_info(logger_kernel,"PID: <%d> Ejecutando Dispositivo:<%s>",pcbRecibida->id,instruccionActual->param1);
         usleep(tiempoRetardo * 1000 * duracion);
        //TODO cambiarEstadoAReady(pcb) podria tener que hacer un post a algun semaforo
        log_info(logger_kernel,"Desbloqueo el proceso <%d> que estaba en <BLOCKED_%s>", pcbRecibida->id, instruccionActual->param1);
        log_info(logger_kernel,"Mando el proceso <%d> a la cola de READY", pcbRecibida->id);
        insertoEnListaReadyBlocked(pcbRecibida);


     //}


    }

}

int levantarHiloAsociadoAColaDispositivo(cola_dispositivo* colaDispositivo){
    pthread_t nuevaCola;
    t_atenderColaDispositivo_args *args = malloc(sizeof(t_atenderColaDispositivo_args));

    args->cola = *colaDispositivo->cola;
    args->nombre= colaDispositivo->nombreDispositivo;
    args->indiceSemaforo = colaDispositivo->indiceSemaforo;
    args->tiempoRetardo = colaDispositivo->tiempoRetardo;
    pthread_create(&nuevaCola, NULL, atenderColaDispositivo, args);
    pthread_detach(nuevaCola);
    return true;
}

void iniciarSemaforoDinamico(sem_t* semaforo, int dim){
    for (int i = 0; i <dim ; ++i) {
        sem_init(&semaforo[i],0,0);
    }
}

int cargarDispositivosIo(){
    listaColasDispositivos = list_create();
    int dim = tamanioArray(cfg_kernel->DISPOSITIVOS_IO);
    char** dispositivos = cfg_kernel->DISPOSITIVOS_IO;
    char** tiempos = cfg_kernel->TIEMPOS_IO;
    for(int i = 0 ; i < dim ; i++){
        cola_dispositivo* colaDispo = malloc(sizeof (cola_dispositivo));
        colaDispo->nombreDispositivo = dispositivos[i];
        colaDispo->tiempoRetardo = atoi(tiempos[i]);
        colaDispo->indiceSemaforo = i;
        colaDispo->cola = queue_create();
        levantarHiloAsociadoAColaDispositivo(colaDispo);
        list_add(listaColasDispositivos,colaDispo);
    }
    estadoBlockIo = listaColasDispositivos;
    return true;
}
//[DISCO,ESCANER,IMPRESORA]; [10,20.100];
//list blockled_io size = 3
// element -> cola_dispositivo
/*
 * typedef struct {
    char* nombreDispositivo;
    int tiempoRetardo;
    t_queue* cola; //Aca guardo las PCB de los procesos
    sem_t semaforo;
}cola_dispositivo;



 HILO DISCO



 */




int init(char *path_config) {
    cfg_kernel = cfg_kernel_start();
    logger_kernel = log_create("kernel.log", "Kernel", true, LOG_LEVEL_INFO);
    if (logger_kernel == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    file_cfg_kernel = iniciar_config(path_config);
    return checkProperties(path_config);

}


void salir(t_log *logger) {
    log_destroy(logger);
}

void cerrar_programa() {

    //matar_hilos();
    cortar_conexiones();
    cerrar_servers();
    config_destroy(file_cfg_kernel);
    log_info(logger_kernel,"TERMINADA_LA_CONFIG");
    log_info(logger_kernel,"TERMINANDO_EL_LOG");
    log_destroy(logger_kernel);
}

void interfaz_function(){
    int s = -1;
    pcb* pcb;
    int socket;
    printf("\nEsta es una interfaz para probar envio de mensajes, presione 0 para mas info, ingrese 9 cuando haya sido cargado una consola \n");
    while(s != 666){

        scanf("%d",&s);
        switch (s) {

            case 0:
                printf("1-Cargar un proceso de prueba \n");
                printf("2-Mensaje a cpu dispatch \n");
                //printf("3-Mensaje a consola, terminar proceso \n");
                printf("3-Mensaje a consola, ingresa un valor y enviamelo \n");
                printf("4-Mensaje a consola, imprime un valor \n");
                printf("5-Enviar un page fault de prueba a memoria \n");
                printf("6-Enviar un acceso a tabla de paginas de prueba a memoria \n");
                printf("7-Inicializar 4 segmentos (tablas de paginas) \n");
                printf("8-Escribir en direccion 0, con valor 25\n");
                printf("9- Escribir en direccion x, con valor y\n");
                printf("10- Leer en direccion 0\n");
                printf("11- Leer en direccion x\n");
                break;
            case 1:
            {
                pcb = queue_pop(estadoExec);
                socket = pcb->id;
                queue_push(estadoExec, pcb);
                break;
            }
            case 2:
                enviarUnJohn(PROCESO_TERMINADO,socket,logger_kernel);
                break;
            case 3:
            {
                enviarOrden(SOLICITAR_VALOR, socket, logger_kernel);
                break;
            }

            case 4:
            {
                uint32_t valor_enviado = 5;
                enviarValor_uint32(valor_enviado,socket,IMPRIMIR_VALOR, logger_kernel);
                log_info(logger_kernel,"El valor que le envie a consola fue: %d",valor_enviado);
                break;
            }
            case 5:
            {
                int array[] = {2,4,1};
                enviar_int_array(array,fd_memoria,PAGE_FAULT,logger_kernel);
                break;
            }
            case 6:
            {
                int array[] = {3,1,0,0};
                enviar_int_array(array,fd_memoria,SOLICITUD_MARCO,logger_kernel);
                break;
            }
            case 7:
            {
                //CANTIDAD ELEM ARRAY, PID, CANTIDAD DE SEGMENTOS
                int array[] = {2, 1, 4};
                enviar_int_array(array, fd_memoria, PROCESO_INICIADO,logger_kernel);
                break;
            }
            case 8:
            {
                int array[] = {3, 1,0, 25};
                enviar_int_array(array, fd_memoria, PEDIDO_ESCRITURA,logger_kernel);
                break;
            }
            case 9:
            {
                uint32_t direccion,valor;
                printf("Ingrese la direccion a escribir\n");
                scanf(&direccion);
                printf("Ingrese el valor a escribir\n");
                scanf(&valor);
                int array[] = {2, direccion, valor};
                enviar_int_array(array, fd_memoria, PEDIDO_ESCRITURA,logger_kernel);
                break;
            }
            case 10:
            {
                enviarValor_uint32(0,fd_memoria, PEDIDO_LECTURA, logger_kernel);
                break;
            }
            case 11:
            {
                uint32_t direccion;
                printf("Ingrese la direccion a escribir\n");
                scanf(&direccion);
                enviarValor_uint32(direccion,fd_memoria, PEDIDO_LECTURA, logger_kernel);
                break;
            }
            default:
                printf("Valor no reconocido \n");
                break;
        }
    }
}


void activar_kernel(){
    //Inicializo los semaforos contadores
    sem_init(&semProcesosEnReady,0,0);
	sem_init(&semProcesosEnRunning,0,1);
	sem_init(&semProcesosEnExit,0,0);
	sem_init(&semProcesosEnNew,0,0);
	//sem_init(&semProcesoInterrumpido,0,0);


    //Inicializo los semaforos mutex
    pthread_mutex_init(&COLANEW, NULL);
	pthread_mutex_init(&COLAREADY, NULL);
	pthread_mutex_init(&COLAEXEC, NULL);
	pthread_mutex_init(&COLABLOCK, NULL);
	pthread_mutex_init(&COLAEXIT, NULL);
	pthread_mutex_init(&PROCDESALOJADO, NULL);

    //Creo las colas de los estados
    estadoNew 	= queue_create();
	//estadoReady = queue_create();
    estadoReadyFifo = list_create();
    estadoReadyRr = list_create();
    estadoBlock = list_create();
    //estadoBlockIo = list_create();
	estadoExec = queue_create();
	estadoExit = queue_create();

    pcbDesalojado=NULL;
    devuelto = 0;

    //Hago la creacion de los hilos
    pthread_t conexion_con_consola;
    pthread_t conexion_con_cpu;
	pthread_t conexion_con_memoria;
	pthread_t planiALargoPlazo;
	pthread_t planiACortoPlazo;
    pthread_t interfaz;
	pthread_create(&conexion_con_consola, NULL,crearServidor, NULL);
	pthread_create(&conexion_con_cpu, NULL, conectarConCPU, NULL);
	pthread_create(&conexion_con_memoria, NULL, conectarConMemoria, NULL);
	pthread_create(&planiALargoPlazo, NULL, planificadorALargoPlazo, NULL); 
	pthread_create(&planiACortoPlazo, NULL,planificadorACortoPlazo, NULL);
    //pthread_create(&interfaz, NULL, interfaz_function,NULL);
    pthread_join(conexion_con_consola, NULL);
    pthread_join(conexion_con_cpu, NULL);
    pthread_join(conexion_con_memoria, NULL);
	pthread_join(planiALargoPlazo, NULL);
	pthread_join(planiACortoPlazo, NULL);


// 
}


