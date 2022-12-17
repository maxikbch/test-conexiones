#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/loggers_configs.h>
#include <commons/log.h>
#include <utils/estructuras.h>
#include <semaphore.h>
#include <PL_Corto.h>
#include <PL_Largo.h>
#include <main.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <Planificacion.h>
#include <comunicacion.h>
extern bool mostrarConfig;
//SEMAFOROS CONTADORES
//extern sem_t semProcesoInterrumpido;
extern sem_t semProcesosEnReady;
extern sem_t semProcesosEnRunning;
extern sem_t semProcesosEnExit;
extern sem_t semProcesosEnNew;

//SEMAFOROS MUTEX
extern pthread_mutex_t COLANEW;
extern pthread_mutex_t COLAREADY;
extern pthread_mutex_t COLAEXEC;
extern pthread_mutex_t COLABLOCK;
extern pthread_mutex_t COLAEXIT;
extern pthread_mutex_t NRODEPROCESO;
extern pthread_mutex_t PROCDESALOJADO;

//COLAS
extern t_queue* estadoNew;
//extern t_queue* estadoReady;
extern t_list* estadoReadyFifo;
extern t_list* estadoReadyRr;
extern t_list* estadoBlock;
extern t_list* estadoBlockIo;

extern t_list* listaColasDispositivos;
extern sem_t* semaforos_io;

extern t_queue* estadoExec;
extern t_queue* estadoExit;

extern int fd_dispatch;
extern int fd_interrupt;
extern int fd_memoria;

extern pcb* pcbDesalojado;//aca se almacena el proceso devuelto por cpu (PENDIENTE DE REVISION)
extern int devuelto;

int argumentosInvalidos(int argc, char *argv[]);
int checkProperties(char *path);
int cargar_configuracion(char *path);

int init(char *path);


int tamanioArray(char ** array);
void iniciarSemaforoDinamico(sem_t* semaforo, int dim);

void salir(t_log *logger);

void cerrar_programa();

void activar_kernel();
int cargarDispositivosIo();





#endif
