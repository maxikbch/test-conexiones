#ifndef INIT_CONSOLE_H_
#define INIT_CONSOLE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utils/loggers_configs.h>
#include<readline/readline.h>
#include <string.h>
#include <sys/stat.h>
//#include <prueba.h>
//#include <protocolo.h>

#include <commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <commons/collections/list.h>

#include <utils/sockets.h>
#include <utils/protocolo.h>
#include <utils/estructuras.h>

extern bool mostrarConfig;

int argumentosInvalidos(int argc, char *argv[]);
void cargar_configuracion(char *path);
void init(char* path);
t_config* iniciar_config(char* path);
void cerrar_programa();
t_list * crear_lista_de_instrucciones(char *path);
int obtener_cantidad_segmentos();
t_list *crear_lista_de_segmentos();




#endif