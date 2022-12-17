//
// Created by utnso on 12/3/22.
//

#include <planificacion.h>
#include <utils/loggers_configs.h>
#include "paginas.h"

pthread_mutex_t MUTEX_MP;
pthread_mutex_t MUTEX_BITMAP;

t_list* procesos;


bool iniciar_planificacion(){
    bool comprobacion =inicializarSemaforos();

    if(!comprobacion){
        log_info(logger_memory,"Archivos para la planificacion y sincronia de memoria, fallaron en la creacion");
        return false;
    }

    log_info(logger_memory,"Archivos para la planificacion y sincronia de memoria, creados exitosamente");
    return true;

}
bool iniciar_estructura_proceso(){
    procesos = list_create();
    return true;
}


bool inicializarSemaforos(){
    pthread_mutex_init(&MUTEX_MP,NULL);
    pthread_mutex_init(&MUTEX_BITMAP,NULL);

    /*
    if(fallo){
        return false
    }
     */
    return true;
}
bool destruirSemaforos(){
    pthread_mutex_destroy(&MUTEX_MP);
    pthread_mutex_destroy(&MUTEX_BITMAP);


    /*
if(fallo){
    return false
}
 */
    return true;
}
int obtenerMarcoSegunAlgoritmo(){
    return -1;
}


void simularRetardoEspacioUsuario(){
    char* message = "Accediendo a espacio de usuario, tiempo estimado de acceso en milisegundos: <%d>";
    char* messageFinal = "Se accedio a espacio de usuario";
    simularRetardo(message,messageFinal,logger_memory,cfg_memory->RETARDO_MEMORIA);
}
void simularRetardoTablaPaginas(){
    char* message = "Accediendo a las tablas de paginas, tiempo estimado de acceso en milisegundos: <%d>";
    char* messageFinal = "Se accedio a a las tablas de paginas";
    simularRetardo(message,messageFinal,logger_memory,cfg_memory->RETARDO_MEMORIA);
}

t_process_memory *buscarProcesoPorPID(int pid) {
    if (list_size(procesos) == NULL) {
        log_error(logger_memory, "No hay ningun proceso creado");
        return NULL;
    }
    for (int i = 0; i < list_size(procesos); ++i) {
        t_process_memory *proceso = list_get(procesos, i);
        if (proceso->pid == pid) {
            proceso = list_get(procesos, i);
            return proceso;
        }
    }
    log_error(logger_memory, "No hay ningun proceso con el PID <%d> asociado", pid);
    return NULL;
}
t_process_memory *buscarYCortarProcesoPorPID(int pid){
    if(list_size(procesos) == 0){
        log_error(logger_memory,"No hay ningun proceso creado");
        return NULL;
    }
    for (int i = 0; i < list_size(procesos) ; ++i) {
        t_process_memory* proceso = list_get(procesos,i);
        if(proceso->pid == pid){
            proceso = list_remove(procesos,i);
            return proceso;
        }
    }
    log_error(logger_memory,"No hay ningun proceso con ese PID asociado");
    return NULL;
}
bool cargarEnSwapPagina(entrada_pagina* pagina){
    if(pagina->bit_modificado == 1){
        void* datos = obtenerDatosDelMarco(pagina->marco);
        uint32_t posicion_swap = pagina->pos_en_swap;
        uint32_t bit_modificado = pagina->bit_modificado;
        actualizarSwap(datos,posicion_swap, bit_modificado);
        bit_modificado = 0;
        return true;
    }
    return false;
}

int clockUnicoPaso(t_process_memory* proceso, bool* eligioVictima){
    int cantidad_marcos = cfg_memory->MARCOS_POR_PROCESO;
    for(int i = 0; i < cantidad_marcos; i++){
        entrada_pagina* paginaActual = queue_pop(proceso->paginasEnMP);
        log_warning(logger_memory, "MARCO <%d> BIT DE USO <%d>", paginaActual->marco, paginaActual->bit_uso);
        if(paginaActual->bit_uso == 0){
            *eligioVictima = true;
            paginaActual->bit_presencia = 0;
            cargarEnSwapPagina(paginaActual);
            queue_push(proceso->paginasEnMP, paginaActual);
            log_warning(logger_memory, "RETORNO MARCO CLOCK");
            return paginaActual->marco;
        } else {
            log_warning(logger_memory, "CAMBIO U A 0 EN CLOCK");
            paginaActual->bit_uso = 0;
        }
        queue_push(proceso->paginasEnMP, paginaActual);
    }
    return false;
}



int ejecutar_algoritmoClock(t_process_memory* proceso){
    bool eligioVictima = false;
    while (1)
    {
        log_warning(logger_memory, "PASO 1 (1ER PASADA)");
        int marco = clockUnicoPaso(proceso, &eligioVictima);
        if(eligioVictima){
            return marco;
        }

        log_warning(logger_memory, "PASO 1 (2DA PASADA)");
        marco = clockUnicoPaso(proceso,&eligioVictima);
        if(eligioVictima){
            return marco;
        }
    }
}

int clockMPaso1(t_process_memory* proceso, bool* eligioVictima){
    // busca U = 0 y M = 0
    int cantidad_marcos = cfg_memory->MARCOS_POR_PROCESO;
    for(int i = 0; i < cantidad_marcos; i++){
        entrada_pagina* paginaActual = queue_pop(proceso->paginasEnMP);
        log_warning(logger_memory, "MARCO <%d> BIT DE USO <%d> BIT DE MOD <%d>", paginaActual->marco, paginaActual->bit_uso, paginaActual->bit_modificado);
        if(paginaActual->bit_uso == 0 && paginaActual->bit_modificado == 0){
            *eligioVictima = true;
            cargarEnSwapPagina(paginaActual);
            paginaActual->bit_presencia = 0;
            queue_push(proceso->paginasEnMP, paginaActual);
            log_warning(logger_memory, "RETORNO MARCO CLOCK");
            return paginaActual->marco;
        }
        queue_push(proceso->paginasEnMP, paginaActual);
    }
    return false;
}
int clockMPaso2(t_process_memory* proceso, bool* eligioVictima, bool setearU0) {
    // busca U = 0 y M = 1
    // y va cambiando los U 1 por U 0
    int cantidad_marcos = cfg_memory->MARCOS_POR_PROCESO;
    for(int i = 0; i < cantidad_marcos; i++){
        entrada_pagina* paginaActual = queue_pop(proceso->paginasEnMP);
        log_warning(logger_memory, "MARCO <%d> BIT DE USO <%d> BIT DE MOD <%d>", paginaActual->marco, paginaActual->bit_uso, paginaActual->bit_modificado);
        if(paginaActual->bit_uso == 0 && paginaActual->bit_modificado == 1){
            *eligioVictima = true;
            cargarEnSwapPagina(paginaActual);
            paginaActual->bit_presencia = 0;
            queue_push(proceso->paginasEnMP, paginaActual);
            log_warning(logger_memory, "RETORNO MARCO CLOCK");
            return paginaActual->marco;
        } else if(setearU0){
            paginaActual->bit_uso = 0;
            log_warning(logger_memory, "CAMBIO U A 0 EN CLOCK");
        }
        queue_push(proceso->paginasEnMP, paginaActual);
    }
    return false;
}


int ejecutar_algoritmoClockM(t_process_memory* proceso){
    bool eligioVictima = false;
    while (1){
        log_warning(logger_memory, "PASO 1 (1ER PASADA)");
        int marco = clockMPaso1(proceso, &eligioVictima);
        if(eligioVictima){
            return marco;
        }

        log_warning(logger_memory, "PASO 2 (1ER PASADA)");
        marco =clockMPaso2(proceso, &eligioVictima, true);
        if(eligioVictima){
            return marco;
        }

        // repite paso 1
        log_warning(logger_memory, "PASO 1 (2DA PASADA)");
        marco =clockMPaso1(proceso, &eligioVictima);
        if(eligioVictima){
            return marco;
        }

        // repite paso 2
        log_warning(logger_memory, "PASO 2 (2DA PASADA)");
        marco =clockMPaso2(proceso, &eligioVictima, false);
        if(eligioVictima){
            return marco;
        }
    }
}

int algoritmoReemplazo(t_process_memory* proceso){

    int marcoAReemplazar;

    if(strcmp(cfg_memory->ALGORITMO_REEMPLAZO, "CLOCK") == 0){
        log_info(logger_memory, "ALGORITMO CLOCK");
        marcoAReemplazar = ejecutar_algoritmoClock(proceso);
    } else if(strcmp(cfg_memory->ALGORITMO_REEMPLAZO, "CLOCK-M") == 0) {
        log_info(logger_memory, "ALGORITMO CLOCK MODIFICADO");
        marcoAReemplazar = ejecutar_algoritmoClockM(proceso);
    } else {
        log_error(logger_memory, "ALGORITMO DE REEMPLAZO INVALIDO");
    }

    return marcoAReemplazar;

}