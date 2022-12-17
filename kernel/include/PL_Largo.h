//
// Created by utnso on 10/6/22.
//

#ifndef KERNEL_PL_LARGO_H
#define KERNEL_PL_LARGO_H

#include <commons/collections/list.h>
#include <malloc.h>
#include <utils/estructuras.h>
#include <utils/loggers_configs.h>
#include <Planificacion.h>
#include <utils/protocolo.h>
#include <commons/collections/queue.h>
#include <init.h>


extern t_list *procesosNuevos;
extern uint32_t nro_proceso;
extern int fd_interrupt;
extern int fd_dispatch;


//Utiliza iniciar_proceso(), devuelve un entero indicando si se pudo crear o no.

//int inicializarProceso(uint32_t fd);

// t_proceso *iniciar_proceso(uint32_t fd, t_list *instrucciones);

void planificadorALargoPlazo();
void enviarProcesosAReady();
void terminarProcesos();

void *atenderProceso(int accepted_fd);
t_proceso* recibir_proceso(uint32_t accepted_fd);

void closure_mostrarListaInstrucciones(instr_t* element);
//pcb *crearPcb(t_proceso *proceso);
pcb* crearPcb(t_proceso *proceso, uint32_t PID);

void crearRegistrosCPU(pcb* pcb);
int procesosCargadosEnBlockedIo();

#endif //KERNEL_PL_LARGO_H
