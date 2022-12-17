#include <comunicacion.h>
#include <interfaz.h>
#include <utils/test_serializacion.h>
int fd_kernel;


void* procesar_conexion(void *void_args) {
    t_procesar_conexion_args *args = (t_procesar_conexion_args *) void_args;
    t_log *logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

    op_code cop;
    while (cliente_socket != -1) {
        if (recv(cliente_socket, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "DISCONNECT!");
            return;
        }
        switch (cop) {
            case DEBUG:
                break;
            case SOLICITAR_VALOR:
            {

                recibirOrden(cliente_socket);
                uint32_t valor = solicitarValor();
                enviarValor_uint32(valor,cliente_socket,SOLICITAR_VALOR, logger_console);
                break;


            }
            case IMPRIMIR_VALOR:
            {
                //uint32_t valor = recibirValor_uint32(cliente_socket);
                //char* valor = recibir_pantalla_teclado(cliente_socket);
                uint32_t valor = recibirValor_uint32(cliente_socket, logger_console);
                imprimirValor(valor);
                enviarOrden(IMPRIMIR_VALOR,cliente_socket, logger_console);
                break;
            }
            case PROCESO_TERMINADO:
            {
                finalizarme();
                return;
            }
            case -1:
                log_error(logger, "Cliente desconectado de %s...", server_name);
                return;
            default:
                log_error(logger, "Algo anduvo mal en el server de %s", server_name);
                log_info(logger, "Cop: %d", cop);
                return;
        }
    }

    log_warning(logger, "El cliente se desconecto de %s server", server_name);
    return;
}



bool generar_conexiones() {
    char* ip;
    char* puerto;
    ip = strdup(cfg_console->IP_KERNEL);
    puerto = strdup(cfg_console->PUERTO_KERNEL);

    log_info(logger_console,"Lei la ip %s", ip);
    log_info(logger_console,"Lei el puerto %s", puerto);

    fd_kernel = crear_conexion(
            logger_console,
            "SERVER KERNEL",
            ip,
            puerto
    );

    free(ip);
    free(puerto);
    return fd_kernel != 0;
}

void cortar_conexiones(){
    liberar_conexion(&fd_kernel);
    log_info(logger_console,"CONEXIONES LIBERADAS");
}



void agregar_a_paquete(t_paquete* paquete, t_proceso* proceso){
    proceso->cantidad_instrucciones = 0;

    void calcularTamanioInstruccion(instr_t *instruccion){ //Calcula el tamaño de la instruccion y la suma al tamaño del buffer del paquete.
        paquete->buffer->size += sizeof(uint8_t) //id
                                 + instruccion->idLength + 1
                                 + (sizeof(uint8_t) * 2) //param1, param2
                                 + instruccion->param1Length + 1
                                 + instruccion->param2Length + 1;

        proceso->cantidad_instrucciones ++;
    }

    list_iterate(proceso->instrucciones, calcularTamanioInstruccion);


    //El tamanio va a ser igual a Cant. Instrucciones + Cant. segmentos + (Cant. segmentos * tamaños de segmentos)
    paquete->buffer->size += 2*sizeof(uint8_t) + sizeof(uint8_t) * (proceso->cantidad_segmentos);


    void* stream = malloc(paquete->buffer->size); //Reservo memoria para el buffer
    int offset=0; //desplazamiento



    void copiarElementos(instr_t *instruccion){ //Copia
        if(strcmp(instruccion->id, "EXIT") == 0){
            memcpy(stream + offset, &instruccion->idLength, sizeof(uint8_t));
            offset += sizeof(uint8_t);
            memcpy(stream + offset, instruccion->id, instruccion->idLength + 1);
            offset += instruccion->idLength +1;
        }
        else{
            memcpy(stream + offset, &instruccion->idLength, sizeof(uint8_t));
            offset += sizeof(uint8_t);
            memcpy(stream + offset, instruccion->id, instruccion->idLength + 1);
            offset += instruccion->idLength +1; // se va a 12 y tendria que tener 8
            memcpy(stream + offset, &instruccion->param1Length, sizeof(uint8_t));
            offset += sizeof(uint8_t); //suma 4 bien 16
            memcpy(stream + offset, instruccion->param1, instruccion->param1Length + 1);
            offset += instruccion->param1Length +1; //suma 3 bien 19
            memcpy(stream + offset, &instruccion->param2Length, sizeof(uint8_t));
            offset += sizeof(uint8_t); //suma 4 bien 23
            memcpy(stream + offset, instruccion->param2, instruccion->param2Length + 1);
            offset += instruccion->param2Length +1;
        }
    }


    //PRIMERO COPIO CANTIDAD DE INSTRUCCIONES (INT)
    memcpy(stream + offset, &proceso->cantidad_instrucciones, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    //SEGUNDO COPIO LAS INSTRUCCIONES
    list_iterate(proceso->instrucciones,copiarElementos);


    //TERCERO COPIO LA CANTIDAD DE SEGMENTOS
    memcpy(stream + offset, &proceso->cantidad_segmentos, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    void copiarElementos_tamanios_segmentos(uint16_t *tamanio_segmento){
        memcpy(stream + offset, tamanio_segmento, sizeof(uint16_t));
        offset += sizeof(uint16_t); //ACA HAY QUE VER PORQUE EN LA ULTIMA VA A QUEDAR UN SIZEOF(UINT8_T) DE MAS
    }

    //CUARTO COPIO LOS TAMANIOS DE SEGMENTOS
    list_iterate(proceso->tam_segmentos,copiarElementos_tamanios_segmentos);


    paquete->buffer->stream = stream;
    log_info(logger_console, "AGREGAR PAQUETE ok");

}

void paquete(t_proceso *proceso, int conexion){
    t_paquete* paquete = crear_paquete(GESTIONAR_CONSOLA_NUEVA);
    log_info(logger_console,"Creo el paquete a enviar");
    agregar_a_paquete(paquete,proceso);
    enviar_paquete(paquete, conexion);
    log_info(logger_console,"Se envio el paquete");
    eliminar_paquete(paquete,logger_console);
}
