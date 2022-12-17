#ifndef COMUNICACION_KERNEL_H_
#define COMUNICACION_KERNEL_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <pthread.h>
#include <gestion_memoria.h>
#include <utils/sockets.h>
#include <utils/protocolo.h>
extern tabla_de_paginas* tablas_de_pagina;

int server_escuchar(t_log *logger, char *server_name, int server_socket);
static void procesar_conexion(void *void_args);
bool generar_conexiones();
void* crearServidor();
void cerrar_servers();
void proceso_iniciado(int cliente_socket);
void proceso_terminado(int cliente_socket);
void page_fault(int cliente_socket);
void pedido_escritura(int cliente_socket);
void pedido_lectura(int cliente_socket);
void solicitud_marco(int cliente_socket);

int* recibir_pid_indice_tabla_y_pagina(int cliente_socket);
int* recibir_indices_tablas_de_paginas(int cliente_socket);
int yaLaCargueEnMP(int cliente_socket, uint32_t pid);

#endif