#ifndef CPU_CPU_H
#define CPU_CPU_H

#include <commons/config.h> //CREO QUE NO VA
#include <commons/log.h>
#include <commons/collections/list.h>
#include <utils/estructuras.h>

#include <commons/string.h>
#include <commons/collections/queue.h>

#include <pthread.h> //CREO QUE NO VA
#include <semaphore.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <math.h>
#include <sys/time.h>

#include <utils/loggers_configs.h>

#include <init_cpu.h>




extern double tiempoInicialCpu;
extern struct timeval tiempoCpu;
extern int interrupcion;
extern int fd_memoria;
extern int fd_kernel;
extern pcb* pcb_actual;
extern sem_t sem_interrupt;




//La TLB es una cola de estas entradas
typedef struct{
	int epid;
    uint32_t esegmento;
	uint32_t epagina;
    uint32_t emarco;
	double ultimaReferencia;
    double instanteCarga;
} entrada_tlb;


void ciclo_instrucciones();
instr_t* fetch(pcb* pcbActual);
char* decode();
void execute();

bool check_interrupt();

void escribir_en_registro(char* registro, uint32_t valor);
uint32_t leer_de_registro(char* registro);

void devolverPcb(op_code codigo, uint32_t accepted_fd);
void devolverPcbPF(pcb_page_fault* pcbPF, op_code codigo, uint32_t accepted_fd);

int direccion_logica();
int direccion_fisica(int marco);

void buscar_marco();
void page_fault();
void recibir_marco_memoria();
void ejecutar_lectura();
void ejecutar_escritura();
void terminar_ejecucion_lectura(uint32_t leido);
void terminar_ejecucion_escritura();

int tamanio_maximo_segmento();
int numero_segmento(int dir_logica);
int desplazamiento_segmento(int dir_logica);
int numero_pagina(int desplazamiento_segmento);
int desplazamiento_pagina(int desplazamiento_segmento);
int cantidad_segmentos_proceso(pcb* pcb);
int tamanio_segmento_proceso(pcb* pcb, int segmento);

void reiniciar_registros();
void reiniciar_valores();

void iniciar_tlb();
bool tlb_sin_entradas();
void limpiar_proceso_tlb(uint32_t id);
bool tlb_completa();
void agregar_a_tlb(int idp, uint32_t nsegmento, uint32_t npagina, uint32_t nmarco);
int consultar_tlb(int id, int seg, int pag);
void reemplazar_entrada(entrada_tlb* entrada);
void algoritmo_fifo(entrada_tlb* entrada);
void algoritmo_lru(entrada_tlb* entrada);
bool menor_instante_carga(entrada_tlb* e1, entrada_tlb* e2);
bool menor_ultima_referencia(entrada_tlb* e1, entrada_tlb* e2);
void limpiar_tlb();
void mostrar_tlb();
void iterador_tlb(entrada_tlb* e);


#endif //CPU_CPU_H
