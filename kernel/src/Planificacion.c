#include <Planificacion.h>
#include <semaphore.h>


//SEMAFOROS CONTADORES
//sem_t semProcesoInterrumpido;
sem_t semProcesosEnReady;
sem_t semProcesosEnRunning;
sem_t semProcesosEnExit;
sem_t semProcesosEnNew;


//SEMAFOROS MUTEX
pthread_mutex_t COLANEW;
pthread_mutex_t COLAREADY;
pthread_mutex_t COLAEXEC;
pthread_mutex_t COLABLOCK;
pthread_mutex_t COLAEXIT;
pthread_mutex_t NRODEPROCESO;
pthread_mutex_t PROCDESALOJADO;

//COLAS
t_queue* estadoNew;
//t_queue* estadoReady;
t_list* estadoReadyFifo;
t_list* estadoReadyRr;
t_list* estadoBlock;
t_list* estadoBlockIo;
t_queue* estadoExec;
t_queue* estadoExit;

t_list *procesosNuevos;
uint32_t nro_proceso;

pcb* pcbDesalojado;
int devuelto;


char* convertirInstruccionesEnSoloIdentificadores(t_list* instrucciones){
    int size = list_size(instrucciones);
    char* instruccionesAsID = string_new();
    for (int i = 0; i <size ; ++i) {
        instr_t * instruccion = list_get(instrucciones,i);
        string_append(&instruccionesAsID,instruccion->id);
        if(!strcmp(instruccion->id,"EXIT") == 0){
            string_append(&instruccionesAsID," " );
        }

    }
    return instruccionesAsID;
}



t_proceso*  recibir_proceso(uint32_t accepted_fd){
    t_proceso* proceso;
    proceso = recibir_paquete(accepted_fd);
    log_info(logger_kernel, "Me llego el siguiente proceso: <%d>", accepted_fd);
    char* instrucciones = convertirInstruccionesEnSoloIdentificadores(proceso->instrucciones);
    log_info(logger_kernel, "Instrucciones : <%s> ",instrucciones);
    //list_iterate(proceso->instrucciones, closure_mostrarListaInstrucciones);
    return proceso;
}



pcb* crearPcb(t_proceso *proceso, uint32_t PID)
{
    pcb *pcbDelProceso= malloc(sizeof(pcb));

    pcbDelProceso->id = PID;

    pcbDelProceso->instr= proceso->instrucciones;
    crearRegistrosCPU(pcbDelProceso); //Aca los registros son un struct ->estructuras.h
    pcbDelProceso->programCounter = 0;
    pcbDelProceso->tablaSegmentos = crearTablaSegmento(pcbDelProceso,proceso); //Aca es tabla de segmentos ->estructuras.h y debemos contar con la info de memoria
    free(proceso);
    log_info(logger_kernel,"PCB del proceso creado, PID <%d> y lo cargamos en <NEW>",pcbDelProceso->id);
    //log_info(logger_kernel,"Falta implementar este log - dentro de crear pcb, a lo sumo mostrar solo PID porque llena de basura");
    //mostrarPcb(pcbDelProceso);
    return pcbDelProceso;
}



void crearRegistrosCPU(pcb* pcb){ //ESTA FUNCION ES SI REGISTROS ES PUNTERO, SI NO HAY Q CAMBIAR LOS SIMBOLOS
    //Creo los registros
    registros_cpu* registros = malloc(sizeof(registros_cpu));
    registros->AX = 0;
    registros->BX = 0;
    registros->CX = 0;
    registros->DX = 0;

    //Los asigno a la pcb pasada
    pcb->registrosCpu = registros;


    //Libero memo dinamica
    //free(registros); --> no estoy seguro de si habría que liberar, porque es justo a lo que apunta
}


t_list* crearTablaSegmento(pcb* pcb,t_proceso *proceso){
    //Los segmentos se liberan con la pcb sino rompe
    t_list *segmento_list = list_create();


    for(int i = 0 ; i < proceso->tam_segmentos->elements_count; i++){
        segmento* seg = malloc(sizeof (segmento));
        uint16_t * obtengoData = list_get(proceso->tam_segmentos,i);
        seg->tamanioSegmento = *obtengoData;
        seg->indiceTablaPaginas = 0;
        list_add(segmento_list,seg);
    }

    return segmento_list;

    //ACA HAY QUE PEDIR A MEMORIA DICHOS NUMEROS

    //Los asigno a la pcb pasada
    //pcb->tablaSegmento = malloc(sizeof(tabla));
    //pcb->tablaSegmento = tabla;
    //Libero memo dinamica


    //free(tabla) Lo comento: Juampi pq este free para mi esta mal ya que no podes liberarla cuando lo creas sino esa memoria se puede pisar
                    //Hay que liberarlo cuando se mata a la PCB
}

void closure_mostrarIndiceTablaPaginas(segmento* segmento){
    printf(" %d ,", (uint16_t) segmento->indiceTablaPaginas);
}

void closure_mostrarListaReady(pcb* unaPcb){
    printf("%d,",unaPcb->id);
}

void mostrarColaDeReady() {
    if (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FEEDBACK") == 0) {
        pthread_mutex_lock(&COLAREADY);
        log_info(logger_kernel,"Cola Ready <FEEDBACK>:");
        log_info(logger_kernel, "Cola de ready RR");
        printf("[");
        list_iterate(estadoReadyRr, closure_mostrarListaReady);
        printf("]\n");

        log_info(logger_kernel, "Cola de ready FIFO");
        printf("[");
        list_iterate(estadoReadyFifo, closure_mostrarListaReady);
        printf("]\n");
        pthread_mutex_unlock(&COLAREADY);
    }
    if (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
        pthread_mutex_lock(&COLAREADY);
        log_info(logger_kernel,"Cola Ready <FIFO>:");
        printf("[");
        list_iterate(estadoReadyFifo, closure_mostrarListaReady);
        printf("]\n");
        pthread_mutex_unlock(&COLAREADY);
    }
    if (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "RR") == 0) {
        pthread_mutex_lock(&COLAREADY);
        log_info(logger_kernel,"Cola Ready <RR>:");
        printf("[");
        list_iterate(estadoReadyRr, closure_mostrarListaReady);
        printf("]\n");
        pthread_mutex_unlock(&COLAREADY);
    }

}











// int cantidadProcesosEnMP() {
//     return cantidadProcesosEnReady() + cantidadProcesosEnBloqueado() + cantidadProcesosEnEjecutando();
// }

// int cantidadProcesosEnReady() {
//     return cantidadProcesosEnUnaLista(procesosReady);
// }

// int cantidadProcesosEnBloqueado() {
//     return cantidadProcesosEnUnaLista(procesosBlock);
// }

// int cantidadProcesosEnEjecutando() {
//     return cantidadProcesosEnUnaLista(procesosExec);
// }

// int cantidadProcesosEnUnaLista(t_list *unaLista) {
//     return list_size(unaLista);
// }
