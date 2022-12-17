#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <commons/log.h>

#include <init.h>
#include <comunicacion.h>
#include <utils/sockets.h>

#include <string.h>
extern char* ip_kernel;
void kernel_activar();

#endif
