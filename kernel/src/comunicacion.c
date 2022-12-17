#include <comunicacion.h>

int fd_dispatch = -1;
int fd_interrupt =-1;
int fd_memoria = -1;
t_list* listaColasDispositivos;
sem_t* semaforos_io;


int fd_mod2 = -1;
char* ip_kernel;
char* puerto_kernel;

pcb* procesoAExit;
pcb* procesoABlocked;
pcb_page_fault* pcbPf;


void insertoEnListaReadyBlocked(pcb* pcbAReady){
    pthread_mutex_lock(&COLAREADY);
    if( (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0) ||  (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "RR") == 0)) {
        list_add(estadoReadyRr,pcbAReady);
        log_info(logger_kernel,"Estado anterior <BlOCKED> estado actual <READY>");
        pthread_mutex_unlock(&COLAREADY);
        mostrarColaDeReady();
    }else{
        list_add(estadoReadyFifo,pcbAReady);
        pthread_mutex_unlock(&COLAREADY);
        mostrarColaDeReady();
    }
    pthread_mutex_unlock(&COLAREADY);
    sem_post(&semProcesosEnReady);
}

void* crearServidor(){
    puerto_kernel=cfg_kernel->PUERTO_ESCUCHA;
    fd_mod2 = iniciar_servidor(logger_kernel, "SERVER KERNEL", ip_kernel, puerto_kernel);
    if (fd_mod2 == 0) {
        log_error(logger_kernel, "Fallo al crear el servidor, cerrando KERNEL");
        return EXIT_FAILURE;
    }
    while (server_escuchar(logger_kernel, "SERVER KERNEL", (uint32_t)fd_mod2));
}

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



            //----------------------------------------CONSOLA-----------------------
            case DEBUG:
            {
                t_package *john = recibirAJohn(cliente_socket,logger_kernel);
                loguearAJohn(logger_kernel, john);
                log_info(logger, "debug");
                break;
            }
            case GESTIONAR_CONSOLA_NUEVA:
            {
/*
                pthread_t atenderProcesoNuevo;
                t_procesar_fd_conexion* conexion = malloc(sizeof(t_procesar_fd_conexion));
                conexion->conexion= cliente_socket;
                pthread_create(&atenderProcesoNuevo,NULL,atenderProceso,conexion);
                pthread_detach(atenderProcesoNuevo);
                */

                atenderProceso(cliente_socket);
                break;

            }
            case IMPRIMIR_VALOR:
            {
                recibirOrden(cliente_socket);
                uint32_t posLista = 0;
                pthread_mutex_lock(&COLABLOCK);
                for(int i = 0 ; estadoBlock->elements_count > i ; i++){
                   pcb* pcbRecibida = list_get(estadoBlock,i);
                   if(pcbRecibida->id == cliente_socket){
                       posLista = i;
                   }
                }
                pcb* pcbAReady = list_remove(estadoBlock,posLista);
                pthread_mutex_unlock(&COLABLOCK);

                log_info(logger_kernel,"Desbloqueo el proceso PID:<%d> que estaba en BLOCKED_PANTALLA", pcbAReady->id);
                log_info(logger_kernel,"Mando el proceso PID:<%d> a la cola de READY", pcbAReady->id);

                insertoEnListaReadyBlocked(pcbAReady);


            break;
            }
            case SOLICITAR_VALOR:
            {
                //TODO cliente_socket seria mi PID.
                uint32_t valor = recibirValor_uint32(cliente_socket, logger_kernel);
                uint32_t posLista = 0;
                pthread_mutex_lock(&COLABLOCK);
                for(int i = 0 ; estadoBlock->elements_count > i ; i++){
                    pcb* pcbRecibida = list_get(estadoBlock,i);
                    if(pcbRecibida->id == cliente_socket){
                        posLista = i;
                    }
                }
                pcb* pcbAReady = list_remove(estadoBlock,posLista);
                pthread_mutex_unlock(&COLABLOCK);
                t_list *listaDeInstrucciones = pcbAReady->instr;
                int apunteProgCounter = pcbAReady->programCounter - 1;
                instr_t* instruccionPantalla = list_get(listaDeInstrucciones, apunteProgCounter);
                char* registroASolicitar = instruccionPantalla -> param2;

                cargarValorRegistro(registroASolicitar,pcbAReady,valor);
                //pcbAReady->programCounter++;

                log_info(logger_kernel,"Del proceso <%d> en el registro <%s> cargue el valor <%d>",pcbAReady->id,registroASolicitar,valor);
                log_info(logger_kernel,"Desbloqueo el proceso <%d> que estaba en BLOCKED_TECLADO", pcbAReady->id);
                log_info(logger_kernel,"Mando el proceso <%d> a la cola de READY", pcbAReady->id);

                insertoEnListaReadyBlocked(pcbAReady);

                break;
            }

            //--------------------------------CPU-------------------------------------------

            case PROCESO_TERMINADO:
                {
                procesoAExit = recibir_pcb(fd_dispatch);
                devuelto = 1;
                sem_post(&semProcesosEnRunning);
                pthread_mutex_lock(&COLAEXEC);
                liberarPcb(queue_pop(estadoExec));
                pthread_mutex_unlock(&COLAEXEC);
                pthread_mutex_lock(&COLAEXIT);
                queue_push(estadoExit,procesoAExit);
                pthread_mutex_unlock(&COLAEXIT);
                log_info(logger_kernel,"Proceso PID:<%d> finalizado", procesoAExit->id);
                log_info(logger_kernel,"PID: <%d> - Estado Anterior: <ESTADO_EXECUTE> - Estado Actual: <EXIT>",procesoAExit->id);
                sem_post(&semProcesosEnExit);

                break;
            }
            case PROCESO_DESALOJADO:{
                    pthread_mutex_lock(&PROCDESALOJADO);
                    devuelto = 1;
                    pcbDesalojado = recibir_pcb(fd_dispatch);
                    log_info(logger, "Proceso <%d> desalojado por FIN DE QUANTUM recibido",pcbDesalojado->id);
                    //sem_post(&semProcesoInterrumpido);

                    pthread_mutex_lock(&COLAREADY);
                    if( (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0) ||  (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FIFO") == 0)) {
                        list_add(estadoReadyFifo,pcbDesalojado);
                        pthread_mutex_unlock(&COLAREADY);
                        log_info(logger_kernel,"PID <%d> Agrego proceso a {COLA FIFO}", pcbDesalojado->id);
                        mostrarColaDeReady();
                }else{
                        list_add(estadoReadyRr,pcbDesalojado);
                        pthread_mutex_unlock(&COLAREADY);
                        log_info(logger_kernel,"PID <%d> Agrego proceso a {COLA RR}", pcbDesalojado->id);
                        mostrarColaDeReady();
                    }
                    //pthread_mutex_unlock(&COLAREADY);
                sem_post(&semProcesosEnRunning);
                sem_post(&semProcesosEnReady);

                    pthread_mutex_lock(&COLAEXEC);
                    liberarPcb(queue_pop(estadoExec));
                    pthread_mutex_unlock(&COLAEXEC);
                    pthread_mutex_unlock(&PROCDESALOJADO);
                    break;
            }

            case BLOCKED_IO:
            {

                pcb * pcbRecibida = recibir_pcb(cliente_socket);
                devuelto = 1;
                uint32_t pcActual =pcbRecibida->programCounter -1;
                instr_t* instr = list_get(pcbRecibida->instr,pcActual);
                log_info(logger_kernel,"CPU me esta mandando PID<%d> BLOCKED IO por instruccion <%s>",pcbRecibida->id,instr->id);
                pthread_mutex_lock(&COLAEXEC);
                liberarPcb(queue_pop(estadoExec));//TODO ACA se tendria que liberar esa memoria o es la misma que la pcb Recibida?
                pthread_mutex_unlock(&COLAEXEC);
                log_info(logger_kernel,"PID: <%d> - Estado Anterior: <ESTADO_EXECUTE> - Estado Actual: <BLOCKED_%s>",pcbRecibida->id,instr->id);
                instr_t* instruccionActual = list_get(pcbRecibida->instr,pcActual);
                char* nombreDispostivo = instruccionActual->param1;

                pthread_mutex_lock(&COLABLOCK);
                for(int i = 0 ; i < list_size(estadoBlockIo) ; i++){
                    cola_dispositivo* colaIo = list_get(estadoBlockIo,i);
                    if((strcmp(colaIo->nombreDispositivo,nombreDispostivo)) == 0){
                        queue_push(colaIo->cola,pcbRecibida);
                        sem_post(&semaforos_io[colaIo->indiceSemaforo]);
                    }
                }
                pthread_mutex_unlock(&COLABLOCK);



                log_info(logger_kernel,"PID: <%d> - Bloqueado por <%s>",pcbRecibida->id,nombreDispostivo);
                sem_post(&semProcesosEnRunning);


                /*
                 * Yo recibo nombre dispositivo. Hago un for con ese nombre y me fijo cual coincide. Yo encuentro ese
                 * nodo. Accedo a ese semaforo y le hago un post.
                 *
                 * Nosotros accedemos a la identidad del hilo por el semaforo.
                 * El hilo lo que hace es hacerle pops a la cola y ejecuta eso.
                 */

                break;
            }
            case BLOCKED_PF: {
                pcb_page_fault* pcbPf = recibir_pcbPf(cliente_socket);
                devuelto = 1;
                pcb* pcbRecibida = pcbPf->pcb;

                log_info(logger_kernel,"Page Fault PID: <%d> - Segmento: <%d> - Pagina: <%d>",pcbPf->pcb->id,pcbPf->segmento,pcbPf->pagina);


                //NECESIDADES -> PCB,INDICE,PAG
                pthread_mutex_lock(&COLABLOCK);
                list_add(estadoBlock, pcbRecibida);
                pthread_mutex_unlock(&COLABLOCK);

                pthread_mutex_lock(&COLAEXEC);
                liberarPcb(queue_pop(estadoExec));
                pthread_mutex_unlock(&COLAEXEC);


                uint32_t* arrayParaMemoria;
                uint32_t arraySize = 3;
                arrayParaMemoria = calloc(arraySize+1,sizeof (uint32_t));
                arrayParaMemoria[0] = arraySize;
                arrayParaMemoria[1] = pcbRecibida->id;
                segmento* seg = list_get(pcbRecibida->tablaSegmentos,pcbPf->segmento);
                arrayParaMemoria[2] = seg->indiceTablaPaginas;
                arrayParaMemoria[3] = pcbPf->pagina;


                enviar_int_array(arrayParaMemoria,fd_memoria,PAGE_FAULT,logger_kernel);
                log_info(logger_kernel,"PID: <%d> - Estado Anterior: <ESTADO_EXECUTE> - Estado Actual: <BLOCKED_PF>",pcbRecibida->id);
                log_info(logger_kernel,"Proceso <%d> enviado a memoria por PF",pcbRecibida->id);
                sem_post(&semProcesosEnRunning);
                break;
            }
            case BLOCKED_TECLADO: {
                devuelto = 1;

                args_peticion_pantalla *args_peticion = malloc(sizeof(args_peticion_pantalla));


                procesoABlocked = recibir_pcb(fd_dispatch);

                args_peticion->pcb = procesoABlocked;

                pthread_mutex_lock(&COLABLOCK);
                list_add(estadoBlock,procesoABlocked);
                pthread_mutex_unlock(&COLABLOCK);

                log_info(logger_kernel,"PID: <%d> - Bloqueado por <TECLADO>",procesoABlocked->id);

                pthread_mutex_lock(&COLAEXEC);
                liberarPcb(queue_pop(estadoExec));
                pthread_mutex_unlock(&COLAEXEC);
                sem_post(&semProcesosEnRunning);


                args_peticion->conexion = procesoABlocked->id;

                pthread_t atenderPeticionTeclado;

                pthread_create(&atenderPeticionTeclado,NULL,peticionTeclado,args_peticion);
                pthread_detach(atenderPeticionTeclado);


                break;
            }

            case BLOCKED_PANTALLA: {
                devuelto = 1;

                args_peticion_pantalla *args_peticion = malloc(sizeof(args_peticion_pantalla));


                procesoABlocked = recibir_pcb(fd_dispatch);

                args_peticion->pcb = procesoABlocked;

                pthread_mutex_lock(&COLABLOCK);
                list_add(estadoBlock,procesoABlocked);
                pthread_mutex_unlock(&COLABLOCK);

                log_info(logger_kernel,"PID: <%d> - Bloqueado por <PANTALLA>",procesoABlocked->id);

                pthread_mutex_lock(&COLAEXEC);
                liberarPcb(queue_pop(estadoExec));
                pthread_mutex_unlock(&COLAEXEC);
                sem_post(&semProcesosEnRunning);

                args_peticion->conexion = procesoABlocked->id;
                pthread_t atenderPeticionPantalla;

                pthread_create(&atenderPeticionPantalla,NULL,peticionPantalla,args_peticion);
                pthread_detach(atenderPeticionPantalla);
                break;
            }

            case ERROR_SIGSEGV:
            {
                devuelto = 1;
                procesoAExit = recibir_pcb(fd_dispatch);
                pthread_mutex_lock(&COLAEXEC);
                liberarPcb(queue_pop(estadoExec));
                pthread_mutex_unlock(&COLAEXEC);
                pthread_mutex_lock(&COLAEXIT);
                queue_push(estadoExit,procesoAExit);
                pthread_mutex_unlock(&COLAEXIT);
                log_info(logger_kernel,"Proceso <%d> finalizado con error por SIGSEGV", procesoAExit->id);
                log_info(logger_kernel,"PID: <%d> - Estado Anterior: <ESTADO_EXECUTE> - Estado Actual: <EXIT>",procesoAExit->id);
                sem_post(&semProcesosEnExit);
                sem_post(&semProcesosEnRunning);
                break;
            }


            //----------------------------------MEMORIA----------------------------------------
            case PROCESO_INICIADO:
            {
                cargamosElProcesoEnReady(cliente_socket);
                break;
            }

            case PROCESO_CARGADO_EN_MP:
            {

                int idProceso = recibirValor_uint32(cliente_socket, logger_kernel);
                log_info(logger_kernel,"Peticion de PF del proceso <%d> resuelta", idProceso);

                int posLista = 0;
                for(int i = 0 ; i < estadoBlock->elements_count; i++){
                    pcb* pcbRecibida = list_get(estadoBlock,i);
                    if(pcbRecibida->id == idProceso){
                        posLista = i;
                    }
                }

                pcb* pcbAReady = list_remove(estadoBlock,posLista);
                log_info(logger_kernel,"Desbloque el proceso <%d> que estaba en BLOCKED_PF", pcbAReady->id);
                log_info(logger_kernel,"Mando el proceso <%d> a la cola de READY", pcbAReady->id);
                insertoEnListaReadyBlocked(pcbAReady);


                break;
            }
            case ENVIO_MARCO_CORRESPONDIENTE:
            {
                int a=recibirValor_uint32(cliente_socket, logger_kernel);
                log_info(logger_kernel,"Me llego este marco: %d",a);
                break;
            }
            case ACCESO_RESULTO_PAGE_FAULT:
            {
                int a= recibirValor_uint32(cliente_socket, logger_kernel);
                log_info(logger_kernel,"Me llego esta pagina: %d",a);
                break;
            }
            case PAGINAS_LIBERADAS:
                {
                    recibirOrden(cliente_socket);
                    pthread_mutex_lock(&COLAEXIT);
                    pcb* procesoEnExit= queue_pop(estadoExit);
                    int pid = procesoEnExit->id;
                    log_info(logger_kernel,"Memoria ya libero el PID <%d> y le hago un POST semProcesosNew",pid);
                    liberarPcb(procesoEnExit);
                    //TODO FALLA EN LIBERAR PCB en las instrucciones
                    pthread_mutex_unlock(&COLAEXIT);
                    sem_post(&semProcesosEnNew);
                    avisarleFinalizacionAConsola(pid);
                    break;
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

void cortar_conexiones(){
    liberar_conexion(&fd_dispatch);
    liberar_conexion(&fd_interrupt);
    liberar_conexion(&fd_memoria);
    log_info(logger_kernel,"CONEXIONES LIBERADAS");
}

void cerrar_servers(){
    close(fd_mod2);
    log_info(logger_kernel,"SERVIDORES CERRADOS");
}



int server_escuchar(t_log *logger, char *server_name, int server_socket) {
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);

    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

void enviarTest(int tipoTest){
    switch (tipoTest) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            break;
    }
}

int atenderInterrupt(){
    if (fd_interrupt == -1){
        return EXIT_FAILURE;
    }
    pthread_t atenderInterrupt;
    t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
    args->log = logger_kernel;
    args->fd = fd_interrupt;
    args->server_name = "ATENDER_INTERRUPT";
    pthread_create(&atenderInterrupt, NULL,procesar_conexion,args);
    pthread_detach(atenderInterrupt);
    return true;
}

int atenderDispatch(){
    if (fd_dispatch == -1){
        return EXIT_FAILURE;
    }
    pthread_t atenderDispatch;
    t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
    args->log = logger_kernel;
    args->fd = fd_dispatch;
    args->server_name = "ATENDER_DISPATCH";
    pthread_create(&atenderDispatch, NULL,procesar_conexion,args);
    pthread_detach(atenderDispatch);
    return true;
}
int atenderMemoria(){
    if (fd_memoria == -1){
        return EXIT_FAILURE;
    }
    pthread_t atenderMemoria;
    t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
    args->log = logger_kernel;
    args->fd = fd_memoria;
    args->server_name = "ATENDER_MEMORIA";
    pthread_create(&atenderMemoria, NULL,procesar_conexion,args);
    pthread_detach(atenderMemoria);
    return true;
}


void* conectarConCPU(){
    int comprobacion = generarConexionesConCPU();
    //if(comprobacion falla){}
    atenderInterrupt();
    atenderDispatch();
    //if(atender interrupt o dispatch fallo){}
}

int generarConexionesConCPU(){

    char* ip;

    ip = strdup(cfg_kernel->IP_CPU);
    log_info(logger_kernel,"Lei la ip %s", ip);

    char* puertoDispatch;
    puertoDispatch = strdup(cfg_kernel->PUERTO_CPU_DISPATCH);

    log_info(logger_kernel,"Lei el puerto %s", puertoDispatch);

    fd_dispatch = crear_conexion(
            logger_kernel,
            "SERVER CPU DISPATCH",
            ip,
            puertoDispatch
    );


    char* puertoInterrupt;
    puertoInterrupt = strdup(cfg_kernel->PUERTO_CPU_INTERRUPT);

    log_info(logger_kernel,"Lei el puerto %s", cfg_kernel->PUERTO_CPU_INTERRUPT);

    fd_interrupt = crear_conexion(
            logger_kernel,
            "SERVER CPU INTERRUPT",
            ip,
            puertoInterrupt
    );


    free(ip);
    free(puertoDispatch);
    free(puertoInterrupt);

    return fd_dispatch != 0 && fd_interrupt != 0;

}

void* conectarConMemoria(){
    int comprobacion = generarConexionesConMemoria();
    atenderMemoria();
}


int generarConexionesConMemoria(){
    char* ip;

    ip = strdup(cfg_kernel->IP_MEMORIA);
    log_info(logger_kernel,"Lei la ip %s", ip);

    char* puerto;
    puerto = strdup(cfg_kernel->PUERTO_MEMORIA);

    log_info(logger_kernel,"Lei el puerto %s", puerto);

    fd_memoria = crear_conexion(
            logger_kernel,
            "SERVER MEMORIA",
            ip,
            puerto
    );


    free(ip);
    free(puerto);

    return fd_memoria != 0;

}

void enviarIntArrayaMemoria(uint32_t *array, op_code op){
    enviar_int_array(array, fd_memoria, op,logger_kernel);
    //aca habria que esperar la respuesta

    //return respuesta;
}

void* peticionPantalla(void *void_args){
    args_peticion_pantalla* args = (args_peticion_pantalla*) void_args;


    pcb *pcbRecibida = args->pcb;
    int conexion = args->conexion; //Es la misma que pcb->id


    t_list *listaDeInstrucciones = pcbRecibida->instr;
    int apunteProgCounter = pcbRecibida->programCounter -1 ;
    instr_t* instruccionPantalla = list_get(listaDeInstrucciones, apunteProgCounter);

    char* registroAImprimir = instruccionPantalla -> param2;
    log_info(logger_kernel,"PID<%d> Le envio a la consola que me imprima el valor del registro {%s}",pcbRecibida->id,registroAImprimir);
    uint32_t valorRegistro = obtenerValorRegistro(registroAImprimir,pcbRecibida);


    enviarValor_uint32(valorRegistro,conexion,IMPRIMIR_VALOR, logger_kernel);

    //enviar_mensaje(registroAImprimir,fd_mod2,IMPRIMIR_VALOR);
    //enviar_pantalla_teclado(registroAImprimir,conexion,IMPRIMIR_VALOR, logger_kernel);
}

void* peticionTeclado(void *void_args){
    args_peticion_pantalla* args = (args_peticion_pantalla*) void_args;

    pcb *pcbRecibida = args->pcb;
    int conexion = args->conexion; //Es la misma que pcb->id
    log_info(logger_kernel,"PID<%d> Le envio a la consola que me escriba el valor para el registro",pcbRecibida->id);

   /*
    t_list *listaDeInstrucciones = pcbRecibida->instr;
    int apunteProgCounter = pcbRecibida->programCounter;
    instr_t* instruccionPantalla = list_get(listaDeInstrucciones, apunteProgCounter);

    char* registroASolicitar = instruccionPantalla -> param2;
    */

    enviarOrden(SOLICITAR_VALOR,conexion, logger_kernel);
    //enviar_mensaje(registroASolicitar,conexion,SOLICITAR_VALOR,logger_kernel);

}

uint32_t obtenerValorRegistro(char* registro,pcb* pcbALeer){
    if(strcmp(registro, "AX") == 0){
        return pcbALeer->registrosCpu->AX;
    }
    if(strcmp(registro, "BX") == 0){
        return pcbALeer->registrosCpu->BX;
    }
    if(strcmp(registro, "CX") == 0){
        return pcbALeer->registrosCpu->CX;
    }
    if(strcmp(registro, "DX") == 0){
        return pcbALeer->registrosCpu->DX;
    }
}

void cargarValorRegistro(char* registro,pcb* pcbALeer,uint32_t valor){
    if(strcmp(registro, "AX") == 0){
        pcbALeer->registrosCpu->AX = valor;
    }
    if(strcmp(registro, "BX") == 0){
        pcbALeer->registrosCpu->BX = valor;
    }
    if(strcmp(registro, "CX") == 0){
        pcbALeer->registrosCpu->CX = valor;
    }
    if(strcmp(registro, "DX") == 0){
        pcbALeer->registrosCpu->DX = valor;
    }
}

void cargamosElProcesoEnReady(int socket_memoria){

    uint32_t *array= recibir_int_array(socket_memoria);

    uint32_t pidRecibido = array[1];
    log_info(logger_kernel,"El PID<%d> ya se encuentra cargado en memoria", array[1]);

    pthread_mutex_lock(&COLANEW);
    pcb* pcbAReady = queue_pop(estadoNew);
    pthread_mutex_unlock(&COLANEW);

    if(pcbAReady->id != pidRecibido){
        log_error(logger_kernel,"TENES UN ERROR EN CARGAR PROCESO EN READY. TENES QUE IMPLEMENTAR EL ESTADO NEW COMO LISTA PID<%d> != [%d]",pcbAReady->id,pidRecibido );
        return EXIT_FAILURE;
    }


    //array[1] = pid
    //array[2] = empiezan los indices de las tablas de paginas
    //Busco el procecso array[1] es mi PID
    //
    /*
    pcb* pcbRecibida;
    //array = {nros elementos siguientes , PID, indices, ...}
    for(int i = 0 ; i<array[0] - 1 ; i++){
        elemento1 de tabla -> array[i+2]
        pcbRecibida->tablaSegmentos
    }
     */


    for(int i = 0 ; i < array[0] - 1 ; i++){
        segmento* seg = list_get(pcbAReady->tablaSegmentos,i);
        seg->indiceTablaPaginas = array[i+2];
    }

    log_info(logger_kernel,"PID: <%d> - Estado Anterior : <ESTADO NEW> - Estado Actual - <ESTADO_READY>",pcbAReady->id);


    if((strcmp(cfg_kernel->ALGORITMO_PLANIFICACION,"FEEDBACK") == 0) || (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION,"RR") == 0) ){
        pthread_mutex_lock(&COLAREADY);
        list_add(estadoReadyRr,pcbAReady);
        pthread_mutex_unlock(&COLAREADY);
        mostrarColaDeReady();
    }
    else{
        pthread_mutex_lock(&COLAREADY);
        list_add(estadoReadyFifo, pcbAReady);
        pthread_mutex_unlock(&COLAREADY);
        mostrarColaDeReady();

    }





    sem_post(&semProcesosEnReady);
}

void avisarleFinalizacionAConsola(int pid){
    log_info(logger_kernel,"PID<%d> Le comunico a CONSOLA la finalizacion del proceso", pid);
    enviarOrden(PROCESO_TERMINADO,pid, logger_kernel);

}
