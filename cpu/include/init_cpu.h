#ifndef INIT_CPU_H_
#define INIT_CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/loggers_configs.h>
#include <comunicacion.h>

extern bool mostrarConfig;

int checkProperties(char *path);

int cargar_configuracion(char *path);

int init(char *path);

void cerrar_programa();

int activar_cpu();

extern sem_t semHayPCBCargada;
extern sem_t semNoEstoyEjecutando;
extern sem_t semEsperandoRespuestaDeMarco;

#endif