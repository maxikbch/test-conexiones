#include <gestion_memoria.h>

void* memoria_contigua;
uint8_t* marcos_ocupados;
uint32_t tamanioBitmap;
uint32_t* puntero_a_bits;
t_bitarray *bitmap_memoria; //array de bits

bool init_memoria(){
    bool primeraComprobacion = iniciar_lista_de_tabla_de_paginas();
    bool segundaComprobacion = inicializar_memoria_contigua();
    bool terceraComprobacion = iniciar_planificacion();
    bool cuartaComprobacion = iniciar_estructura_proceso();
    bool sextaComprobacion = crearSwap();
    bool comprobacionFinal = !primeraComprobacion ||  !segundaComprobacion || !terceraComprobacion || !cuartaComprobacion || !sextaComprobacion;
    if(comprobacionFinal){
        log_error(logger_memory,"Error al inicializar la memoria, tablas de paginas, y/o iniciar la planificacion");
        return false;
    }
    return true;

}



int inicializar_memoria_contigua(){
    memoria_contigua = malloc(cfg_memory->TAM_MEMORIA);
    memset(memoria_contigua, 0, sizeof(memoria_contigua));

    uint32_t cantidad_de_marcos_en_mp = cfg_memory->TAM_MEMORIA / cfg_memory->TAM_PAGINA;

    if(cfg_memory->TAM_MEMORIA % cfg_memory->TAM_PAGINA != 0){
        cantidad_de_marcos_en_mp ++;
    }

    tamanioBitmap = cantidad_de_marcos_en_mp;

    puntero_a_bits = malloc(cantidad_de_marcos_en_mp);
    bitmap_memoria = bitarray_create_with_mode(puntero_a_bits,cantidad_de_marcos_en_mp, LSB_FIRST);
    //msync(bitmap_memoria->bitarray, cantidad_de_marcos_en_mp, MS_SYNC);

    for(int i=0; i<cantidad_de_marcos_en_mp; i++){
        bitarray_clean_bit(bitmap_memoria, i);
    }

    log_info(logger_memory, "Memoria Principal creada.");
    return true; //TODO CONTROLAR SI REALMENTE SE CREO CORRECTAMENTE
}



int obtener_marco_disponible(t_process_memory* proceso){

    int cantidadEntradasProceso = queue_size(proceso->paginasEnMP);
    bool procesoSuperaMaxEntrada = cfg_memory->ENTRADAS_POR_TABLA <= cantidadEntradasProceso;

    //si hay bits disponibles, buscar bit en 0 (false), setearlo en 1 (true) y devolver la posicion del bit.

    pthread_mutex_lock(&MUTEX_BITMAP);
    if(!procesoSuperaMaxEntrada){
    log_warning(logger_memory, "NO ENTRO AL CLOCK");
        for(int i = 0; i < tamanioBitmap; i++){
            if(!bitarray_test_bit(bitmap_memoria, i)){
                bitarray_set_bit(bitmap_memoria, i);

                pthread_mutex_unlock(&MUTEX_BITMAP);
                return i;
            }

        }
    }
        pthread_mutex_unlock(&MUTEX_BITMAP);
        log_warning(logger_memory, "ENTRO AL CLOCK");
        return algoritmoReemplazo(proceso);
        //en caso contrario, aplica algoritmo para obtener un marco

        //---> EN TEORIA, NO DEBERIA SER NECESARIO LIMPIAR EL MARCO, CON SOBREESCRIBIR TA BIEN

}
void* obtenerDatosDelMarco(int marco){
    uint32_t tamanioPagina = cfg_memory->TAM_PAGINA;
    void* datos = malloc(sizeof(tamanioPagina));
    pthread_mutex_lock(&MUTEX_MP);
    memcpy(datos,memoria_contigua+marco*tamanioPagina,sizeof (tamanioPagina));
    pthread_mutex_unlock(&MUTEX_MP);
    return datos;
}

void liberar_marco(uint32_t posicion_marco){

    pthread_mutex_unlock(&MUTEX_MP);

    //Estoy liberando el marco, que tiene un tamaño de un uint32_t asi que calculo que un uint32_t
    memset(memoria_contigua + posicion_marco*cfg_memory->TAM_PAGINA, 0, cfg_memory->TAM_PAGINA);
    pthread_mutex_unlock(&MUTEX_MP);
    log_trace(logger_memory,"Libere el marco <%d>", posicion_marco);
    //PRIMERO DEBERIA LIBERARSE EL MARCO Y DESPUÉS EL BIT QUE MARCA QUE ESTA LIBRE
    pthread_mutex_lock(&MUTEX_BITMAP);
    bitarray_clean_bit(bitmap_memoria, posicion_marco);
    pthread_mutex_unlock(&MUTEX_BITMAP);
    log_debug(logger_memory,"Ahora esta disponible para usar el marco <%d>", posicion_marco);
}

int buscarMarcoAsociadoConDireccion(uint32_t posicion){
    int marco= posicion;//posicion/cfg_memory->TAM_PAGINA;
    /*
    if(posicion % cfg_memory->TAM_PAGINA != 0){
        marco++;
    }
     */
    return marco;
}
entrada_pagina* buscarEntradaPaginaPorMarco(int marco){
    for (int i = 0; i < cantidad_tablas ; i++) {
        if(tablas_de_pagina[i].ocupado == 1) {
            for (int j = 0; j <cfg_memory->ENTRADAS_POR_TABLA ; j++) {
                if(tablas_de_pagina[i].paginas[j].marco== marco){
                    return &tablas_de_pagina[i].paginas[j];
                }
            }
        }
    }
    log_error(logger_memory,"No se encontro la entrada asociada al marco");
    return NULL;
}




void prenderBitModificadoSegunPosicion(uint32_t posicion){
    int marco = buscarMarcoAsociadoConDireccion(posicion);
    entrada_pagina* entrada_pagina = buscarEntradaPaginaPorMarco(marco);
    entrada_pagina->bit_modificado=1;
}
void prenderBitUsoSegunMarco(uint32_t marco)
{
    //int marco = buscarMarcoAsociadoConDireccion(posicion);
    entrada_pagina* entrada_pagina = buscarEntradaPaginaPorMarco(marco);
    if(entrada_pagina == NULL){
        log_error(logger_memory, "Abortando busqueda");
        return;
    }
    entrada_pagina->bit_uso=1;
}
void prenderAmbosBitsSegunEntrada(entrada_pagina* entradaPagina) {
    entradaPagina->bit_uso=1;
    entradaPagina->bit_modificado=1;
}


//LA POSICION SE MULTIPLICARIA POR EL MARCO? O SABEMOS EXACTAMENTE A DONDE TENEMOS QUE IR?
bool escribir_memoria_contigua(uint32_t posicion, uint32_t valor, uint32_t marco) {

    //int marco = buscarMarcoAsociadoConDireccion(posicion);
    entrada_pagina * entradaPagina = buscarEntradaPaginaPorMarco(marco);
    if(entradaPagina == NULL){
        return false;
    }
    pthread_mutex_lock(&MUTEX_MP);
    simularRetardoEspacioUsuario();
    memcpy(memoria_contigua + posicion, &valor, sizeof(uint32_t));
    pthread_mutex_unlock(&MUTEX_MP);
    prenderAmbosBitsSegunEntrada(entradaPagina);
    log_info(logger_memory, "Se escribio en la direccion: <%d>, el valor: <%d> de tamanio <%d>",posicion,valor,sizeof (uint32_t));

    /*
     if(fallo){return false}
     */

    return true;

}



uint32_t leer_memoria_contigua(uint32_t posicion, uint32_t pid, uint32_t marco) {
    uint32_t valor;
    pthread_mutex_lock(&MUTEX_MP);
    memcpy(&valor, memoria_contigua + posicion, sizeof(uint32_t));
    pthread_mutex_unlock(&MUTEX_MP);

    prenderBitUsoSegunMarco(marco);
    return valor;
}


void cargarPaginaEnProceso(t_process_memory * proceso,entrada_pagina *entrada) {
    //A lo sumo comprobacion
    queue_push(proceso->paginasEnMP, entrada);
}
bool cargarDatosPaginaEnMarco(void* pagina, int marco){

    pthread_mutex_lock(&MUTEX_MP);
    memcpy(memoria_contigua + marco*cfg_memory->TAM_PAGINA, pagina, sizeof(cfg_memory->TAM_PAGINA));
    pthread_mutex_unlock(&MUTEX_MP);
    log_info(logger_memory,"Se cargo datos de la pagina en el marco: <%>", marco);
    return true;
}


int cargarEnMP(uint32_t pid, uint32_t indiceTablaDePaginas, uint32_t indicePagina){
    bool tablaNoValida =indiceTablaDePaginas> cantidad_tablas || indiceTablaDePaginas < 0;
    bool paginaNoValida = indicePagina > cfg_memory->ENTRADAS_POR_TABLA || indicePagina <0;

    if(tablaNoValida || paginaNoValida){
        log_error(logger_memory, "Se ingreso el indice de tabla de paginas: <%d> y la pagina <%d>, su combinacion es invalida", indiceTablaDePaginas, indicePagina);
        return false;
    }
    entrada_pagina * entrada = buscarEntrada(indiceTablaDePaginas, indicePagina);
    if(entrada == NULL){
        log_error(logger_memory,"Pagina: <%d> del Segmento: <%d> del Proceso: <%d> no inicializada",indicePagina,indiceTablaDePaginas,pid);
        return NULL;
    }

    t_process_memory * proceso = buscarProcesoPorPID(pid);
    int marcoDisponible = obtener_marco_disponible(proceso);
    entrada->marco= marcoDisponible;
    void* datos = traerPaginaDeSwap(entrada->pos_en_swap);
    cargarDatosPaginaEnMarco(datos, marcoDisponible);
    entrada->bit_presencia=1;
    cargarPaginaEnProceso(proceso, entrada);
    log_info(logger_memory,"Para la pagina <%d>, del segmento: <%d>, se cargo el marco: <%d>",indicePagina,indiceTablaDePaginas, marcoDisponible);
    //TODO: FREE PELIGROSO
    free(datos);
    return true;
}

t_list* paginasEnMemoriaDeProceso(t_process_memory* proceso){
    return NULL;
}
