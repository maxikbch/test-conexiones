#include <PL_Largo.h>
#include <init.h>

typedef struct{
    int conexion;
}t_procesar_fd_conexion;

void planificadorALargoPlazo(){
     pthread_t hiloProcesosAReady;
	 pthread_t  hiloTerminarProcesos;
	 pthread_create(&hiloProcesosAReady,NULL,enviarProcesosAReady,NULL);
	 pthread_create(&hiloTerminarProcesos,NULL,terminarProcesos,NULL);

	 pthread_detach(hiloProcesosAReady);
	 pthread_detach(hiloTerminarProcesos);
}

 void enviarProcesosAReady(){
	 while(1)
	 {
		sem_wait(&semProcesosEnNew);
		pthread_mutex_lock(&COLAREADY);
		pthread_mutex_lock(&COLAEXEC);
		pthread_mutex_lock(&COLABLOCK);
		uint32_t gradoDeMultiProgActual= list_size(estadoBlock)+ //TODO considerar todos los estados block EJ estadoBlockPf//ESTA Mal me toma los dispositivos
										 list_size(estadoReadyFifo)+
										 list_size(estadoReadyRr)+
										 queue_size(estadoExec)+
                                         procesosCargadosEnBlockedIo();
			if(gradoDeMultiProgActual < cfg_kernel->GRADO_MAX_MULTIPROGRAMACION &&
				queue_size (estadoNew)>0)
			{
				pthread_mutex_lock(&COLANEW);
				pcb *procesoAReady = queue_peek(estadoNew);
				pthread_mutex_unlock(&COLANEW);


                //Enviamos el proceso a memoria para que lo cargue

                uint32_t* arrayParaMemoria;
                uint32_t arraySize = 2;
                arrayParaMemoria = calloc(arraySize+1,sizeof (uint32_t));
                arrayParaMemoria[0] = arraySize;
                arrayParaMemoria[1] = procesoAReady->id;
                uint32_t cantidadSegmentos = list_size(procesoAReady->tablaSegmentos);
                arrayParaMemoria[2] = cantidadSegmentos;

                //mostrarIntArray(arrayParaMemoria,"%d, ",logger_kernel);
                log_info(logger_kernel,"Proceso <%d> enviado a memoria para ser cargado",procesoAReady->id);
                enviar_int_array(arrayParaMemoria,fd_memoria,PROCESO_INICIADO,logger_kernel);

                /* POR EL MOMENTO A LA PARTE DE MEMORIA NO LE DAMOS BOLA
				uint32_t tablaDePaginas= obtenerTablaDePagina(procesoAReady);
				if(tablaDePaginas <0){
					perror("Error al asignar memoria al proceso");
					return EXIT_FAILURE;
				}
				procesoAReady->tablaDePaginas = tablaDePaginas;
                */



                /*

				if((strcmp(cfg_kernel->ALGORITMO_PLANIFICACION,"FEEDBACK") == 0) || (strcmp(cfg_kernel->ALGORITMO_PLANIFICACION,"RR") == 0) ){
					queue_push(estadoReadyRr,procesoAReady);
				}
				else{
					queue_push(estadoReadyFifo, procesoAReady);
				}
			
				sem_post(&semProcesosEnReady);
                 */
			}
         pthread_mutex_unlock(&COLAREADY);
         pthread_mutex_unlock(&COLAEXEC);
         pthread_mutex_unlock(&COLABLOCK);
		}
	 }

void terminarProcesos(){
    //TODO terminarProceso pendiente a implementar (es liberar la PCB )
    while(1){
        sem_wait(&semProcesosEnExit);
        pthread_mutex_lock(&COLAEXIT);
        pcb* procesoEnExit;
        procesoEnExit = queue_peek(estadoExit);


        log_info(logger_kernel,"Estado EXIT le aviso a MEMORIA que libere la memoria tomada por el proceso <%d>" , procesoEnExit->id);
        //Se lo envio a memoria para que libere esa memoria
        enviarValor_uint32(procesoEnExit->id,fd_memoria,PROCESO_TERMINADO, logger_kernel);
        pthread_mutex_unlock(&COLAEXIT);

        //Libero de kernel la memoria tomada por esa PCB



    }



}
//void *atenderProceso(void *void_args)
void *atenderProceso(int accepted_fd)
{
    //t_procesar_fd_conexion *args = (t_procesar_fd_conexion *) void_args;
    //int accepted_fd = args->conexion;
    pthread_mutex_lock(&COLANEW);
	t_proceso *nuevoProceso = recibir_proceso(accepted_fd);
	pcb* nuevoPcb= crearPcb(nuevoProceso, accepted_fd);

    //Esto no va por el hecho de que me tiene que permitir cargarlo el grado de multiprogramacion

    //uint32_t* arrayConPIDYCantidadIndices = calloc(3,sizeof(uint32_t));
    //arrayConPIDYCantidadIndices[0] = 2;
    //arrayConPIDYCantidadIndices[1] = nuevoPcb->id;
    //arrayConPIDYCantidadIndices[2] = list_size(nuevoPcb->tablaSegmentos);
    //enviar_int_array(arrayConPIDYCantidadIndices, fd_memoria, PROCESO_INICIADO, logger_kernel);


    queue_push(estadoNew,nuevoPcb);
    //log_info(logger_kernel,"Se crea el proceso PID:<%d> en NEW", nuevoPcb->id);
    pthread_mutex_unlock(&COLANEW);
    sem_post(&semProcesosEnNew);
}


int procesosCargadosEnBlockedIo() {
    int cantidad = 0;
    for(int i = 0 ; i< list_size(estadoBlockIo); i++){
        cola_dispositivo* colaIO = list_get(estadoBlockIo,i);
        cantidad = cantidad + queue_size(colaIO->cola);
    }
    return cantidad;
}

