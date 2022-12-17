//
// Created by utnso on 11/7/22.
//

#ifndef CONSOLE_COMUNICACION_H
#define CONSOLE_COMUNICACION_H

#include <utils/sockets.h>
#include <utils/protocolo.h>
#include <utils/estructuras.h>
#include <pthread.h>
void* procesar_conexion(void *void_args);
bool generar_conexiones();
void cortar_conexiones();
//t_paquete* crear_paquete(op_code codigo_de_operacion); //En protocolo.h
void agregar_a_paquete(t_paquete*, t_proceso* proceso);
//void* serializar_paquete(t_paquete* paquete, int bytes); //En protocolo.h
//void enviar_paquete(t_paquete* paquete, int socket_cliente); //En protocolo.h
//void eliminar_paquete(t_paquete* paquete); //En protocolo.h
void paquete(t_proceso*, int);




#endif //CONSOLE_COMUNICACION_H
