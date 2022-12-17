//
// Created by utnso on 12/3/22.
//

#ifndef MEMORY_PLANIFICACION_H
#define MEMORY_PLANIFICACION_H
#include <pthread.h>
#include <stdbool.h>
#include <utils/estructuras.h>
#include <auxiliar.h>







typedef struct{
    uint32_t pid;
    t_list * indicesTablasPaginas;
    t_queue* paginasEnMP;
}t_process_memory;



bool iniciar_planificacion();
bool inicializarSemaforos();
bool iniciar_estructura_proceso();
bool destruirSemaforos();
int obtenerMarcoSegunAlgoritmo();
void simularRetardoEspacioUsuario();
void simularRetardoTablaPaginas();
t_process_memory *buscarProcesoPorPID(int pid);
t_process_memory *buscarYCortarProcesoPorPID(int pid);
int ejecutar_algoritmoClock(t_process_memory* proceso);
int ejecutar_algoritmoClockM(t_process_memory* proceso);
int algoritmoReemplazo(t_process_memory* proceso);

#endif //MEMORY_PLANIFICACION_H