#ifndef MEMORY_GESTION_MEMORIA_H
#define MEMORY_GESTION_MEMORIA_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <utils/loggers_configs.h>
#include <pthread.h>
#include <commons/bitarray.h>
#include <malloc.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <paginas.h>
#include <planificacion.h>
extern pthread_mutex_t MUTEX_MP;
extern pthread_mutex_t MUTEX_BITMAP;
extern pthread_mutex_t MUTEX_ID;
extern t_list* procesos;
extern tabla_de_paginas* tablas_de_pagina;


bool init_memoria();
int inicializar_memoria_contigua();

bool escribir_memoria_contigua(uint32_t posicion, uint32_t valor, uint32_t marco);
void prenderBitUsoSegunMarco(uint32_t marco);
void prenderBitModificadoSegunPosicion(uint32_t posicion);
void prenderAmbosBitsSegunEntrada(entrada_pagina* entradaPagina);
bool cargarDatosPaginaEnMarco(void* pagina, int marco);
void* obtenerDatosDelMarco(int marco);
void liberar_marco(uint32_t posicion_marco);
uint32_t leer_memoria_contigua(uint32_t posicion, uint32_t pid, uint32_t marco);
int cargarEnMP(uint32_t pid, uint32_t indiceTablaDePaginas, uint32_t indicePagina);


#endif //MEMORY_GESTION_MEMORIA_H