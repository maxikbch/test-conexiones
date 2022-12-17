//
// Created by utnso on 12/2/22.
//

#ifndef MEMORY_SWAP_H
#define MEMORY_SWAP_H

#include <init.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <sys/mman.h>
#include <commons/string.h>
#include <fcntl.h>


extern void* memoria_contigua;
uint32_t crearPaginaEnSwap();
void actualizarSwap(void* datos, uint32_t posicion_swap, uint32_t bit_modificado);
char * liberarPagina(uint32_t posicion);
void* traerPaginaDeSwap(uint32_t posSwap);
bool crearSwap();
void eliminarSwap();
//extern t_list* procesos;

#endif