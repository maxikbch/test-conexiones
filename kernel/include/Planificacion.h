#ifndef KERNEL_PLANIFICACION_H
#define KERNEL_PLANIFICACION_H

#include <init.h>
 #include <commons/collections/list.h>
 #include <utils/estructuras.h>
 #include <commons/collections/queue.h>
#include <utils/protocolo.h>
#include <commons/string.h>


t_proceso*  recibir_proceso(uint32_t accepted_fd);
pcb* crearPcb(t_proceso *proceso, uint32_t PID);
void crearRegistrosCPU(pcb* pcb);
t_list* crearTablaSegmento(pcb* pcb,t_proceso* proceso);
void closure_mostrarListaReady(pcb* unaPcb);
void mostrarColaDeReady();


// int cantidadProcesosEnMP();

// int cantidadProcesosEnReady();

// int cantidadProcesosEnBloqueado();

// int cantidadProcesosEnEjecutando();

// int cantidadProcesosEnUnaLista(t_list *unaLista);


 #endif //KERNEL_PLANIFICACION_H
