#include <PL_Corto.h>


void *planificadorACortoPlazo(){
    int utilizarFifo = -1;
    int utilizarRr = -1;
    int utilizarFeedback = -1;

	 utilizarFifo = strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FIFO");
	 utilizarRr = strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "RR");
     utilizarFeedback = strcmp(cfg_kernel->ALGORITMO_PLANIFICACION, "FEEDBACK");

		if(utilizarFifo == 0){
				//Ejecutar FIFO.
			log_info(logger_kernel,"Planificador FIFO.");
			planificadorFifo();
		}

		if(utilizarRr == 0){
				//Ejecuta RR.
			log_info(logger_kernel,"Planificador RR.");
			planificadorRr();
		} 

        if(utilizarFeedback == 0){
                //Ejecutar Feedback
            log_info(logger_kernel,"Planificador Feedback.");
            planificadorFeedback();
        }  
    
}
void* esperarRespuestaCPU(){
    op_code cop;
    if (recv(fd_dispatch, &cop, sizeof(op_code), MSG_WAITALL) != sizeof(op_code)) {
        log_info(logger_kernel, "DISCONNECT!");
    }
    pcb* unaPcb = recibir_pcb(fd_dispatch);
    mostrarPcb(unaPcb);

}

void planificadorFifo(){
	while (1)
		{
            //TODO te falta sincronizar bien los semaforos. ya que a veces se pone a ejecutar antes que este cargado
			sem_wait(&semProcesosEnReady);
			sem_wait(&semProcesosEnRunning);
            pthread_mutex_lock(&COLAREADY);
            if(list_size(estadoReadyRr) <= 0) { //Ponemos esto ya que RR tiene prioridad de ejecucion
			if(list_size(estadoReadyFifo) > 0) {
				pcb* elemEjecutar = list_remove(estadoReadyFifo,0);
                pthread_mutex_unlock(&COLAREADY);
                pthread_mutex_lock(&COLAEXEC);
				queue_push(estadoExec,elemEjecutar);
                pthread_mutex_unlock(&COLAEXEC);
                log_info(logger_kernel,"PID: <%d> - Estado Anterior : <ESTADO_READY> - Estado Actual - <ESTADO_EXECUTE>",elemEjecutar->id );
                enviar_paquete_pcb(elemEjecutar,fd_dispatch,PCB, logger_kernel);
                //pthread_t esperarRespuesta;
                //pthread_create(&esperarRespuesta,NULL,esperarRespuestaCPU,NULL);

			}
			
		}
            }

}


void planificadorRr(){
    pthread_t hiloQuantum;
    while (1)
    {
        sem_wait(&semProcesosEnReady);
        sem_wait(&semProcesosEnRunning);
        pthread_mutex_lock(&COLAREADY);
        if(list_size(estadoReadyRr) > 0) {
            pcb* elemEjecutar = list_remove(estadoReadyRr,0);
            pthread_mutex_unlock(&COLAREADY);
            pthread_mutex_lock(&COLAEXEC);
            queue_push(estadoExec,elemEjecutar);
            pthread_mutex_unlock(&COLAEXEC);
            log_info(logger_kernel,"PID: <%d> - Estado Anterior : <ESTADO_READY> - Estado Actual - <ESTADO_EXECUTE>",elemEjecutar->id );
            devuelto = 0;
            enviar_paquete_pcb(elemEjecutar,fd_dispatch, PCB,logger_kernel);


            //Hasta aca era igual a FIFO, ya que toma un proceso de la cola y lo envia por dispatch a CPU
            //Ahora comenzamos a verificar el tema del quantum
			pthread_create(&hiloQuantum,NULL,finDeQuantum,(void*)elemEjecutar); //TODO creo que hay que sacarle el elemEjecutar
			pthread_join(hiloQuantum,NULL);
            
            
        }
    }
}
	
	
void finDeQuantum(void* void_args){
	pcb* proceso = (pcb*) void_args;
    usleep(cfg_kernel->QUANTUM_RR * 1000);
	if(devuelto == 0 ) { //Chequeamos que no se haya desalojado antes que se cumpla el quantum
        enviarOrden(INTERRUPCION,fd_interrupt, logger_kernel);
        log_info(logger_kernel,"PID:<%d> Interrupcion por fin de quantum enviada a CPU", proceso->id);
        //sem_wait(&semProcesoInterrumpido); //blocked y pcbdesalojado le dan signal

    if(devuelto == 1){ // Si despues de la interrupcion se desalojo
        log_info(logger_kernel,"No hay procesos en CPU");
    }
	}
    
}

void planificadorFeedback() {
    //TODO planificador Multiples colas DEJALO PARA EL FINAL

    while (1) {
        sem_wait(&semProcesosEnReady);
        sem_wait(&semProcesosEnRunning);
        if (list_size(estadoReadyRr) > 0) { //APLICAMOS RR
            pthread_mutex_lock(&COLAREADY);
            pcb *elemEjecutar = list_remove(estadoReadyRr, 0);
            pthread_mutex_lock(&COLAEXEC);
            queue_push(estadoExec, elemEjecutar);
            pthread_mutex_unlock(&COLAEXEC);
            pthread_mutex_unlock(&COLAREADY);
            log_info(logger_kernel, "PID: <%d> - Estado Anterior : <ESTADO_READY> - Estado Actual - <ESTADO_EXECUTE>",
                     elemEjecutar->id);
            devuelto = 0;
            enviar_paquete_pcb(elemEjecutar, fd_dispatch, PCB, logger_kernel);


            //Hasta aca era igual a FIFO, ya que toma un proceso de la cola y lo envia por dispatch a CPU
            //Ahora comenzamos a verificar el tema del quantum
            pthread_t hiloQuantum;
            pthread_create(&hiloQuantum, NULL, finDeQuantum,
                           (void *) elemEjecutar); //TODO creo que hay que sacarle el elemEjecutar
            pthread_join(hiloQuantum, NULL);
            continue;
        }
        if(list_size(estadoReadyFifo) > 0) {
            pcb* elemEjecutar = list_remove(estadoReadyFifo,0);
            queue_push(estadoExec,elemEjecutar);
            log_info(logger_kernel,"PID: <%d> - Estado Anterior : <ESTADO_READY> - Estado Actual - <ESTADO_EXECUTE>",elemEjecutar->id );
            enviar_paquete_pcb(elemEjecutar,fd_dispatch,PCB, logger_kernel);
            continue;
        }else{
            log_error(logger_kernel,"NO SE PQ NO ENTRA NI A RR NI A FIFO");
        }

    }
}
//Probamos haciendolo To do en el mismo algoritmo







