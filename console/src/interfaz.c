//
// Created by utnso on 10/5/22.
//
#include <interfaz.h>



void imprimirValor(uint32_t valor){
    char* message= "Procesando mensaje...";
    char* messageFinal = "Mensaje procesado";
    simularRetardo(message,messageFinal,logger_console,cfg_console->TIEMPO_PANTALLA);
    log_info(logger_console,"El valor enviado por kernel es: <%d>",valor);
}

//Realmente guarda la info?
uint32_t solicitarValor(){
    //TODO
    uint32_t valor;
    log_info(logger_console,"Ingrese un valor a enviar a kernel para el registro: ");
    scanf("%d",&valor);
    return valor;
}

int esperarOrdenes(){
        if (fd_kernel == -1){
            return EXIT_FAILURE;
        }
        pthread_t esperarOrdenes;
        t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger_console;
        args->fd = fd_kernel;
        args->server_name = "ESPERAR_ORDENES";
        pthread_create(&esperarOrdenes, NULL,procesar_conexion,args);
        pthread_join(esperarOrdenes,NULL);
        return true;

}

void finalizarme(){
    log_info(logger_console,"PROCESO FINALIZADO");
    log_info(logger_console,"SALIENDO Y LIBERANDO ESTRUCTURAS");
}