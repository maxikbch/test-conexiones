#ifndef INIT_MEMORY_H_
#define INIT_MEMORY_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <utils/loggers_configs.h>
#include <comunicacion.h>
#include <commons/collections/list.h>
extern bool mostrarConfig;
/*
extern t_log *logger_memory;
extern t_config *file_cfg_memory;
extern t_config_memory *cfg_memory;
*/

int checkProperties(char* path);
int cargar_configuracion(char *path);
int init(char* path);
void cerrar_programa();

#endif