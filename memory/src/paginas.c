#include <paginas.h>




///////////////////////////////////////////////////
////INICIALIZACION////
///////////////////////////////////////////////////

void mostrarPagina(entrada_pagina *pagina);

int* inicializar_estructuras_proceso(int* array){
    int* arrayPosiciones;
    //Se almacenan indices tablas de paginas
    arrayPosiciones = buscar_tablas_disponibles(array[1], array[2]);

    return arrayPosiciones;
}

bool iniciar_lista_de_tabla_de_paginas() {
    tablas_de_pagina = malloc(sizeof(tabla_de_paginas) * cantidad_tablas);

    for (int i = 0; i < cantidad_tablas; i++) {
        tablas_de_pagina[i].ocupado = 0;

    }
    log_info(logger_memory,"Listas con tablas de paginas creada");
    return true; //CONTROLAR SI REALMENTE SE INICIALIZA
}

char * iniciar_pagina(entrada_pagina *pagina) {
    pagina->marco =-1;
    pagina->bit_presencia = 0;
    pagina->bit_uso = 0;
    pagina->bit_modificado = 0;
    pagina->pos_en_swap = crearPaginaEnSwap();
    return string_itoa(pagina->pos_en_swap);
}
/*
void mostrarPagina(entrada_pagina *pagina) {

}
*/

void crear_paginas_necesarias(tabla_de_paginas* tabla) {
    tabla->paginas = malloc(sizeof(entrada_pagina) * (cfg_memory->ENTRADAS_POR_TABLA));
    char* resultadoInicializacion = string_new();
    for (int i = 0; i < cfg_memory->ENTRADAS_POR_TABLA; i++) {
        string_append(&resultadoInicializacion,iniciar_pagina(&tabla->paginas[i]));
        if(i+1 < cfg_memory->ENTRADAS_POR_TABLA){
            string_append(&resultadoInicializacion, ", ");
        }

    }
    log_debug(logger_memory,"Se crearon las paginas con las posiciones en swap: <%s> respectivamente", resultadoInicializacion);
    free(resultadoInicializacion);
}

/////////////////////////////////////////////////////

///////////////////////////////////////////////////
////BUSQUEDA////
///////////////////////////////////////////////////


int *buscar_tablas_disponibles(int pid, int cantidad_necesaria) {

    int *arrayPosiciones = malloc(sizeof(int) * (cantidad_necesaria + 1));
    arrayPosiciones[0] = cantidad_necesaria;

    for (int i = 0; i < cantidad_necesaria; i++) {
        for (int j = 0; j < cantidad_tablas; j++) {
            if (tablas_de_pagina[j].ocupado == 0) {
                tablas_de_pagina[j].ocupado = 1;
                arrayPosiciones[i + 1] = j;
                crear_paginas_necesarias(&tablas_de_pagina[j]);
                int tamanio= cfg_memory->ENTRADAS_POR_TABLA;
                log_debug(logger_memory,"Accion: CREACION - PID: <%d> - SEGMENTO: <%d> - TAMANIO: <%d> paginas",pid,j,tamanio);
                break;
            }
        }
    }

    return arrayPosiciones;
}


entrada_pagina* buscarEntrada(uint32_t indiceTablaDePaginas, uint32_t indicePagina){
    entrada_pagina * entradaEncontrada = &tablas_de_pagina[indiceTablaDePaginas].paginas[indicePagina];
    uint8_t bitPresencia = entradaEncontrada->bit_presencia;
    int marco = entradaEncontrada->marco;
    int posicionEnSwap = entradaEncontrada->pos_en_swap;
    uint8_t  bitModificado = entradaEncontrada->bit_modificado;
    uint8_t  bitUso = entradaEncontrada->bit_uso;

    bool primerComprobacionGarbage = bitPresencia>1 || bitPresencia<0;
    bool segundaComprobacionGarbage = marco<0 && marco != -1;
    bool tercerComprobacionGarbage = posicionEnSwap<0;
    bool cuartaComprobacionGarbage = bitModificado>1 || bitModificado<0;
    bool quintaComprobacionGarbage = bitUso>1 || bitUso<0;
    if(primerComprobacionGarbage || segundaComprobacionGarbage || tercerComprobacionGarbage || cuartaComprobacionGarbage || quintaComprobacionGarbage){
        log_error(logger_memory, "Se esta solicitando una entrada no inicializada");
        return NULL;
    }
    return entradaEncontrada;
}

uint32_t buscarMarcoAsociado(uint32_t indiceTablaDePaginas, uint32_t indicePagina){
    if(tablas_de_pagina[indiceTablaDePaginas].ocupado ==0){
        log_error(logger_memory,"Indice de tabla de paginas invalido: %d", indiceTablaDePaginas);
        return -2;
    }
    entrada_pagina* entrada= buscarEntrada(indiceTablaDePaginas, indicePagina);

    if(entrada->bit_presencia == 0){

        return -1;
    }
    uint32_t marco = entrada->marco;
    return marco;
}

////////////////

///////////////////////////////////////////////////
////LIBERACION DE ESTRUCTURAS////
///////////////////////////////////////////////////


bool liberarPaginas(int indiceTablaPaginas, int pid){
    int tamanio= cfg_memory->ENTRADAS_POR_TABLA;
    char* resultadoLiberacion = string_new();
    for (int j = 0; j <tamanio ; ++j) {
        entrada_pagina * pagina = &tablas_de_pagina[indiceTablaPaginas].paginas[j];
        if(pagina->marco != -1){
            liberar_marco(pagina->marco);
        }

        char* liberacion = liberarPagina(pagina->pos_en_swap);
        string_append(&resultadoLiberacion,liberacion);

        if(j+1 < tamanio){
            string_append(&resultadoLiberacion, ", ");
        }



    }
    log_trace(logger_memory,"Accion: DESTRUCCION - PID: <%d> - Segmento: <%d>, - Paginas con posiciones en swap: <%s>", pid,indiceTablaPaginas, resultadoLiberacion);
    free(tablas_de_pagina[indiceTablaPaginas].paginas);
    free(resultadoLiberacion);
}


void liberar_estructuras_proceso(int pid) {

    t_process_memory * proceso =buscarYCortarProcesoPorPID(pid);
    t_list * indices = proceso->indicesTablasPaginas;
    //int tamanio= cfg_memory->ENTRADAS_POR_TABLA;
    while (list_size(proceso->indicesTablasPaginas) > 0) {
        int i = list_size(proceso->indicesTablasPaginas)-1;
        int* indice =list_remove(indices, i);
        if(*indice < 0 || *indice > cantidad_tablas){
            log_error(logger_memory,"Indice de tabla de paginas invalido, no se pudo liberar las estructuras del proceso <%d>", *indice);
            return;
        }
        liberarPaginas(*indice, pid);

        tablas_de_pagina[*indice].ocupado= 0;


    }


    log_trace(logger_memory,"Mi estructura auxiliar proceso en memoria con PID: <%d> fue liberada",proceso->pid );
    free(proceso);
}
