#ifndef CPU_MAIN_H
#define CPU_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <init_cpu.h>
#include <utils/sockets.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <comunicacion.h>

extern int fd_memoria;
extern int fd_dispatch;
extern int fd_interrupt;
extern char *ip_cpu;

void cerrarPrograma();


#endif //CPU_MAIN_H
