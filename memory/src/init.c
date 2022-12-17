#include <init.h>

int checkProperties(char* path)
{
    t_config *config = config_create(path);
    if (config == NULL)
    {
        log_error(logger_memory, "Ocurrió un error al intentar abrir el archivo config");
        return false;
    }

    char *properties[] = {
            "PUERTO_ESCUCHA",
            "TAM_MEMORIA",
            "TAM_PAGINA",
            "ENTRADAS_POR_TABLA",
            "RETARDO_MEMORIA",
            "ALGORITMO_REEMPLAZO",
            "MARCOS_POR_PROCESO",
            "RETARDO_SWAP",
            "PATH_SWAP",
            NULL};

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties))
    {
        log_error(logger_memory, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}

void cerrar_programa() {

    //matar_hilos();
    cerrar_servers();
    eliminarSwap();
    log_info(logger_memory,"TERMINADA_LA_CONFIG");
    log_info(logger_memory,"TERMINANDO_EL_LOG");
    log_destroy(logger_memory);
}

bool cargar_configuracion_con_log(t_config* file_cfg_memory){
    cfg_memory->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_memory, "PUERTO_ESCUCHA"));
    log_info(logger_memory, "PUERTO_ESCUCHA Cargada Correctamente: %s", cfg_memory->PUERTO_ESCUCHA);

    cfg_memory->TAM_MEMORIA = config_get_int_value(file_cfg_memory, "TAM_MEMORIA");
    log_info(logger_memory, "TAM_MEMORIA Cargado Correctamente: %d", cfg_memory->TAM_MEMORIA);

    cfg_memory->TAM_PAGINA = config_get_int_value(file_cfg_memory, "TAM_PAGINA");
    log_info(logger_memory, "TAM_PAGINA cargado correctamente: %d", cfg_memory->TAM_PAGINA);

    cfg_memory->ENTRADAS_POR_TABLA = config_get_int_value(file_cfg_memory, "ENTRADAS_POR_TABLA");
    log_info(logger_memory, "ENTRADAS_POR_TABLA Cargada Correctamente: %d", cfg_memory->ENTRADAS_POR_TABLA);

    cfg_memory->RETARDO_MEMORIA = config_get_int_value(file_cfg_memory, "RETARDO_MEMORIA");
    log_info(logger_memory, "RETARDO_MEMORIA Cargado Correctamente: %d", cfg_memory->RETARDO_MEMORIA);


    cfg_memory->ALGORITMO_REEMPLAZO = strdup(config_get_string_value(file_cfg_memory, "ALGORITMO_REEMPLAZO"));
    log_info(logger_memory, "ALGORITMO_REEMPLAZO cargado correctamente: %s", cfg_memory->ALGORITMO_REEMPLAZO);

    cfg_memory->MARCOS_POR_PROCESO = config_get_int_value(file_cfg_memory, "MARCOS_POR_PROCESO");
    log_info(logger_memory, "MARCOS_POR_PROCESO Cargada Correctamente: %d", cfg_memory->MARCOS_POR_PROCESO);

    cfg_memory->RETARDO_SWAP = config_get_int_value(file_cfg_memory, "RETARDO_SWAP");
    log_info(logger_memory, "RETARDO_SWAP correctamente: %d", cfg_memory->RETARDO_SWAP);


    cfg_memory->PATH_SWAP = strdup(config_get_string_value(file_cfg_memory, "PATH_SWAP"));
    log_info(logger_memory, "PATH_SWAP Cargada Correctamente: %s", cfg_memory->PATH_SWAP);
    if(!config_has_property(file_cfg_memory, "TAMANIO_SWAP")){
        log_warning(logger_memory,"Se esta inicializando sin SWAP");
    }else{
        cfg_memory->TAMANIO_SWAP = config_get_int_value(file_cfg_memory, "TAMANIO_SWAP");
        log_info(logger_memory, "TAMANIO_SWAP Cargado Correctamente: %d", cfg_memory->TAMANIO_SWAP);
    }

    return true;
}
bool cargar_configuracion_perse(t_config* file_cfg_memory){
    cfg_memory->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_memory, "PUERTO_ESCUCHA"));
    cfg_memory->TAM_MEMORIA = config_get_int_value(file_cfg_memory, "TAM_MEMORIA");
    cfg_memory->TAM_PAGINA = config_get_int_value(file_cfg_memory, "TAM_PAGINA");
    cfg_memory->ENTRADAS_POR_TABLA = config_get_int_value(file_cfg_memory, "ENTRADAS_POR_TABLA");
    cfg_memory->RETARDO_MEMORIA = config_get_int_value(file_cfg_memory, "RETARDO_MEMORIA");
    cfg_memory->ALGORITMO_REEMPLAZO = strdup(config_get_string_value(file_cfg_memory, "ALGORITMO_REEMPLAZO"));
    cfg_memory->MARCOS_POR_PROCESO = config_get_int_value(file_cfg_memory, "MARCOS_POR_PROCESO");
    cfg_memory->RETARDO_SWAP = config_get_int_value(file_cfg_memory, "RETARDO_SWAP");
    cfg_memory->PATH_SWAP = strdup(config_get_string_value(file_cfg_memory, "PATH_SWAP"));
    if(!config_has_property(file_cfg_memory, "TAMANIO_SWAP")){
        log_warning(logger_memory,"Se esta inicializando sin SWAP");
    }else{
        cfg_memory->TAMANIO_SWAP = config_get_int_value(file_cfg_memory, "TAMANIO_SWAP");
    }


    return true;
}


int cargar_configuracion(char *path)
{

    file_cfg_memory = config_create(path);

   if(mostrarConfig){
       cargar_configuracion_con_log(file_cfg_memory);
   }else{
       cargar_configuracion_perse(file_cfg_memory);
   }

    log_info(logger_memory, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_memory);
    return true;
}

int init(char* path)
{

    cfg_memory = cfg_memory_start();
    logger_memory = log_create("memory.log", "Memory", true, LOG_LEVEL_TRACE);

    if(logger_memory== NULL){
        printf("No pude crear el logger");
        return false;
    }
    file_cfg_cpu = iniciar_config(path);



    return checkProperties(path);
}

typedef struct{
    uint32_t *socket1;
    uint32_t *cod_op1;
}arg_struct;

void *handshake(uint32_t socket){
    void *buffer;

    uint32_t cod_op= HANDSHAKE_CPU;
    uint32_t offset=0;
    uint32_t tamanio= sizeof(uint32_t)*3;
    buffer= malloc(tamanio);
    memcpy(buffer,&cod_op,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer+offset,&cfg_memory->TAM_PAGINA,sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(buffer+offset,&cfg_memory->ENTRADAS_POR_TABLA,sizeof(uint32_t));

    send(socket, buffer, tamanio, 0);

    free(buffer);
}