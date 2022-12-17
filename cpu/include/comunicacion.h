#ifndef COMUNICACION_KERNEL_H_
#define COMUNICACION_KERNEL_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>


#include <utils/sockets.h>
#include <utils/protocolo.h>
#include <cpu.h>

extern uint32_t entradasPorTabla;
extern uint32_t tamanioDePagina;
extern bool tengoQueEsperarRespuestaContinuar;


void *crearServidorDispatch();

void *crearServidorInterrupt();

bool atenderMemoria();
bool generarConexionesConMemoria();

void *conectarConMemoria();

void cortar_conexiones();

void cerrar_servers();

void procesoTerminado();

void procesoBloqueado();

void procesoDesalojado();

void cpuVacia();


bool generar_conexiones();

int server_escuchar(t_log *logger, char *server_name, int server_socket);

void ejecucionPcbTest(pcb* pcb_actual,int cliente_socket,op_code codigoOperacion, uint32_t programCounterDeseado);


#endif
