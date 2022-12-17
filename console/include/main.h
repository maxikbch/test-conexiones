#ifndef CONSOLE_MAIN_H
#define CONSOLE_MAIN_H


#include <init.h>
#include <stdlib.h>
#include<stdio.h>
#include<readline/readline.h>

#include <commons/log.h>
#include<commons/string.h>
#include<commons/config.h>


#include <utils/sockets.h>
#include <interfaz.h>
#include <comunicacion.h>
#include <signal.h>
#include <utils/estructuras.h>
bool liberarProceso();

extern char* path_pseudo;
extern char* path_config;

void handle_sigint(int sig);


#endif //CONSOLE_MAIN_H
