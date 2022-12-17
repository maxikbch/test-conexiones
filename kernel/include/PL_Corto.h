//
// Created by utnso on 10/6/22.
//

#ifndef KERNEL_PL_CORTO_H
#define KERNEL_PL_CORTO_H

#include <init.h>
#include <utils/sockets.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <semaphore.h>
#include <utils/loggers_configs.h>
#include <utils/estructuras.h>
#include <Planificacion.h>


extern int fd_interrupt;
extern int fd_dispatch;

void *planificadorACortoPlazo();
void planificadorFifo();
void planificadorRr();
void finDeQuantum();
void planificadorFeedback();
extern int fd_interrupt;

#endif //KERNEL_PL_CORTO_H