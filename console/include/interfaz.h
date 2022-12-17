//
// Created by utnso on 10/5/22.
//

#ifndef CONSOLE_INTERFAZ_H
#define CONSOLE_INTERFAZ_H
#include <stdlib.h>
#include <stdio.h>
#include <comunicacion.h>
#include <pthread.h>

void imprimirValor(uint32_t valor);
//Realmente guarda la info?
uint32_t solicitarValor();
int esperarOrdenes();
void finalizarme();
extern int fd_kernel;

#endif //CPU_INTERFAZ_H
