#include <init_cpu.h>
#include <time.h>

double tiempoInicialCpu;
struct timeval tiempoCpu;


int checkProperties(char *path_config) {
    // hay 2 argumentos, el path de el psudocodigo y el path de la config
    // config valida
    t_config *config = config_create(path_config);
    if (config == NULL) {
        log_error(logger_cpu, "Ocurrió un error al intentar abrir el archivo config");
        return false;
    }

    char *properties[] = {
            "ENTRADAS_TLB",
            "REEMPLAZO_TLB",
            "RETARDO_INSTRUCCION",
            "IP_MEMORIA",
            "PUERTO_MEMORIA",
            "PUERTO_ESCUCHA_DISPATCH",
            "PUERTO_ESCUCHA_INTERRUPT",
            NULL};

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties)) {
        log_error(logger_cpu, "Propiedades faltantes en el archivo de configuracion");
        return false;
    }

    config_destroy(config);

    return true;
}
bool cargar_config_perse(t_config* file_cfg_cpu){
    cfg_cpu->IP_MEMORIA = strdup(config_get_string_value(file_cfg_cpu, "IP_MEMORIA"));
    cfg_cpu->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_MEMORIA"));
    cfg_cpu->PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_ESCUCHA_DISPATCH"));
    cfg_cpu->PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_ESCUCHA_INTERRUPT"));
    cfg_cpu->RETARDO_INSTRUCCION = config_get_int_value(file_cfg_cpu, "RETARDO_INSTRUCCION");
    cfg_cpu->ENTRADAS_TLB = config_get_int_value(file_cfg_cpu, "ENTRADAS_TLB");
    cfg_cpu->REEMPLAZO_TLB = strdup(config_get_string_value(file_cfg_cpu, "REEMPLAZO_TLB"));
    return true;
}

bool cargar_config_perse_con_log(t_config* file_cfg_cpu){
    cfg_cpu->IP_MEMORIA = strdup(config_get_string_value(file_cfg_cpu, "IP_MEMORIA"));
    log_info(logger_cpu, "IP_MEMORIA Cargada Correctamente: %s", cfg_cpu->IP_MEMORIA);

    cfg_cpu->PUERTO_MEMORIA = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_MEMORIA"));
    log_info(logger_cpu, "PUERTO_MEMORIA Cargado Correctamente: %s", cfg_cpu->PUERTO_MEMORIA);

    cfg_cpu->PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_ESCUCHA_DISPATCH"));
    log_info(logger_cpu, "PUERTO_ESCUCHA_DISPATCH cargado correctamente: %s", cfg_cpu->PUERTO_ESCUCHA_DISPATCH);

    cfg_cpu->PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(file_cfg_cpu, "PUERTO_ESCUCHA_INTERRUPT"));
    log_info(logger_cpu, "PUERTO_ESCUCHA_INTERRUPT cargado correctamente: %s", cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);

    cfg_cpu->RETARDO_INSTRUCCION = config_get_int_value(file_cfg_cpu, "RETARDO_INSTRUCCION");
    log_info(logger_cpu, "RETARDO_INSTRUCCION cargado correctamente: %d", cfg_cpu->RETARDO_INSTRUCCION);

    cfg_cpu->ENTRADAS_TLB = config_get_int_value(file_cfg_cpu, "ENTRADAS_TLB");
    log_info(logger_cpu, "ENTRADAS_TLB Cargada Correctamente: %d", cfg_cpu->ENTRADAS_TLB);

    cfg_cpu->REEMPLAZO_TLB = strdup(config_get_string_value(file_cfg_cpu, "REEMPLAZO_TLB"));
    log_info(logger_cpu, "REEMPLAZO_TLB Cargado Correctamente: %s", cfg_cpu->REEMPLAZO_TLB);
    return true;
}

int cargar_configuracion(char *path) {
    config_destroy(file_cfg_cpu);  //Destruye antes?

    file_cfg_cpu = config_create(path);
    if(mostrarConfig){
        cargar_config_perse_con_log(file_cfg_cpu);
    }else{
        cargar_config_perse(file_cfg_cpu);
    }



    //CARGAR EL TIEMPO DE INICIO DE CPU PARA TENER DISPONIBLE PARA ALGORITMOS DE REEMPLAZO
    gettimeofday(&tiempoCpu, NULL);
    tiempoInicialCpu = tiempoCpu.tv_sec;

    //CARGAR LA TLB 
    iniciar_tlb();

    
    log_info(logger_cpu, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_cpu);
    return true;
}

void cerrar_programa() {

    //matar_hilos();
    cortar_conexiones();
    cerrar_servers();
    config_destroy(file_cfg_cpu);

    //LIBERAR TLB
    //free(cola_tlb);

    log_info(logger_cpu,"TERMINADA_LA_CONFIG");
    log_info(logger_cpu, "TERMINANDO_EL_LOG");
    log_destroy(logger_cpu);
}

int init(char *path_config) {
    cfg_cpu = cfg_cpu_start();
    logger_cpu = log_create("cpu.log", "Cpu", true, LOG_LEVEL_INFO);
    if (logger_cpu == NULL) {
        printf("No pude crear el logger");
        return false;
    }
    file_cfg_cpu = iniciar_config(path_config);
    return checkProperties(path_config);
}

int activar_cpu() {
    //sem_init(&semHayPCBCargada, 0, 0);
    //sem_init(&semNoEstoyEjecutando,0,1);
    //sem_init(&semEsperandoRespuestaDeMarco, 0, 0);
    return true;
}
