#ifndef COMUNICACION_KERNEL_H_
#define COMUNICACION_KERNEL_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <PL_Largo.h>

#include <utils/sockets.h>
#include <utils/protocolo.h>
#include <utils/test_serializacion.h>
#include <utils/estructuras.h>

extern sem_t* semaforos_io;

typedef struct{
    pcb* pcb;
    int conexion;
}args_peticion_pantalla;

void* crearServidor();
int server_escuchar(t_log *logger, char *server_name, int server_socket);
void cortar_conexiones();
void cerrar_servers();
void* conectarConCPU();
void* conectarConMemoria();
int generarConexionesConMemoria();
int generarConexionesConCPU();
int atenderInterrupt();
int atenderDispatch();
void enviarIntArrayaMemoria(uint32_t *array, op_code op);
void* peticionPantalla(void *void_args);
void* peticionTeclado(void *void_args);
uint32_t obtenerValorRegistro(char* registro,pcb* pcbALeer);
void cargarValorRegistro(char* registro,pcb* pcbACargar,uint32_t valor);
void insertoEnListaReadyBlocked(pcb* pcbAReady);
void cargamosElProcesoEnReady(int socket_memoria);
void avisarleFinalizacionAConsola(int pid);
#endif
