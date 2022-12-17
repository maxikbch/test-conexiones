#include <init.h>
#include <comunicacion.h>
t_log *logger_console;
t_config *file_cfg_console;
t_config_console *cfg_console;
char* path_pseudo;
char* path_config;
t_list *inst_list;

int argumentosInvalidos(int argc, char *argv[])
{
    // hay 2 argumentos, el path de el psudocodigo y el path de la config
    if (argc < 2)
    {
        log_error(logger_console, "Se esperaba: [CONFIG_PATH]\n");
        return 1;
    }
    path_pseudo = argv[2];

    // config valida
    t_config *config = config_create(path_config);
    if (config == NULL)
    {
        log_error(logger_console, "Ocurrió un error al intentar abrir el archivo config");
        return 1;
    }


    char *properties[] = {
            "IP_KERNEL",
            "PUERTO_KERNEL",
            "SEGMENTOS",
            "TIEMPO_PANTALLA",
            NULL};

    // Falta alguna propiedad
    if (!config_has_all_properties(config, properties))
    {
        log_error(logger_console, "Propiedades faltantes en el archivo de configuracion");
        return 1;
    }
    log_info(logger_console,"Liberando config");
    config_destroy(config);

    return 0;
}
//No funciona a menos que le pasemos size... lo cual es un problema porque no lo conocemos, de momento.
void log_array(char** array, t_log* logger, int size){

    //int size = sizeof(array) / sizeof(array[0]);

    for (int i = 0; i < size; ++i) {
        log_info(logger, "Se cargo el segmento: %s", array[i]);
    }
}

bool cargar_configuracion_con_log(t_config* file_cfg_console){
    cfg_console->IP_KERNEL = strdup(config_get_string_value(file_cfg_console, "IP_KERNEL"));
    log_info(logger_console, "IP Cargada Correctamente: %s", cfg_console->IP_KERNEL);

    cfg_console->PUERTO_KERNEL = strdup(config_get_string_value(file_cfg_console, "PUERTO_KERNEL"));
    log_info(logger_console, "Puerto Cargado Correctamente: %s", cfg_console->PUERTO_KERNEL);

    cfg_console->TIEMPO_PANTALLA = config_get_int_value(file_cfg_console, "TIEMPO_PANTALLA");
    log_info(logger_console, "EL tiempo en pantalla fue cargado correctamente: %d", cfg_console->TIEMPO_PANTALLA);
    cfg_console->SEGMENTOS = config_get_array_value(file_cfg_console, "SEGMENTOS");
    return true;
}
bool cargar_configuracion_perse(t_config* file_cfg_console){
    cfg_console->IP_KERNEL = strdup(config_get_string_value(file_cfg_console, "IP_KERNEL"));
    cfg_console->PUERTO_KERNEL = strdup(config_get_string_value(file_cfg_console, "PUERTO_KERNEL"));
    cfg_console->TIEMPO_PANTALLA = config_get_int_value(file_cfg_console, "TIEMPO_PANTALLA");
    cfg_console->SEGMENTOS = config_get_array_value(file_cfg_console, "SEGMENTOS");
    return true;
}

void cargar_configuracion(char *path)
{


    file_cfg_console = config_create(path);


    if(mostrarConfig){
        cargar_configuracion_con_log(file_cfg_console);
    }else{
        cargar_configuracion_perse(file_cfg_console);
    }

    char** segmentos = cfg_console->SEGMENTOS;
    int comprobacion = segmentos[0];
    int tamanio = 0;
    while(comprobacion != NULL){
        tamanio++;
        comprobacion = segmentos[tamanio];
    }
    if(mostrarConfig){
        cfg_console->TAMANIO_LISTA_SEGMENTOS=tamanio;
        log_info(logger_console,"Los tamanios de los segmentos fueron cargados correctamente, Cantidad :%d", cfg_console->TAMANIO_LISTA_SEGMENTOS);
    }else{
        cfg_console->TAMANIO_LISTA_SEGMENTOS=tamanio;
    }

    log_info(logger_console, "Archivo de configuracion cargado correctamente");
    config_destroy(file_cfg_console);
}

//Habría que pensarlo de nuevo si genera un problema al ser varias consolas...
void init(char* path_config)
{
    /*
    if(cfg_console == NULL){
    }
     */
    cfg_console = cfg_console_start();
    logger_console = log_create("console.log", "Console", true, LOG_LEVEL_INFO);
    if(logger_console== NULL){
        printf("No pude crear el logger");
        exit(2);
    }

    file_cfg_console = iniciar_config(path_config);
    //la funcion de aca arriba genera leaks still reacheables pero como es generado por las commons
    //no se si se puede solucionar o si hace verdaderamente falta
}




int tamanioArray(char ** array){
    int n=0;
    for(int i=0 ;*(array+i)!= NULL; i++)
        n++;
    return n;
}


void closure_instrucciones(char *line) {
    char** inst_and_param = string_split(line, " ");    //divido la linea por " " y me crea un vector y me lo mete segun las divisiones
    instr_t *single_inst = malloc(sizeof(instr_t));     //reservo memoria para estructura instr_t
    single_inst->idLength= strlen(inst_and_param[0]);
    single_inst->id= malloc(single_inst->idLength + 1);
    memcpy(&single_inst->id,&inst_and_param[0],single_inst->idLength +1);        //copio el id de la instruccion
    //single_inst->nroDeParam= tamanioArray(inst_and_param)-1;     //guardo la cantidad de parametros

    if(strcmp(single_inst->id, "EXIT") == 0){
        single_inst->param1Length = 0;
        single_inst ->param1 = NULL;
        single_inst->param2Length = 0;
        single_inst->param2 = NULL;
        list_add(inst_list, single_inst); //agrego la instruccion a la lista
    }
    else{
        single_inst->param1Length = strlen(inst_and_param[1]);
        single_inst->param1 = malloc(single_inst->param1Length + 1);
        memcpy(&single_inst->param1,&inst_and_param[1],single_inst->param1Length +1);
        single_inst->param2Length = strlen(inst_and_param[2]);
        single_inst->param2 = malloc(single_inst->param2Length + 1);
        memcpy(&single_inst->param2,&inst_and_param[2],single_inst->param2Length +1);

        list_add(inst_list, single_inst); //agrego la instruccion a la lista
    }


}

t_list * crear_lista_de_instrucciones(char *path) {
    FILE* file = fopen(path, "r");
    struct stat stat_file;
    stat(path, &stat_file);
    if (file == NULL) {
        return NULL;
    }
    char* buffer = calloc(1, stat_file.st_size + 1);
    fread(buffer, stat_file.st_size, 1, file);
    for (int i = 1; !feof(file); i++) {
        fread(buffer, stat_file.st_size, 1, file);
    }

    char **inst_per_line = string_split(buffer, "\n"); //divido el archivo leido por lineas


    inst_list = list_create();    //creo la lista de instrucciones


    string_iterate_lines(inst_per_line, closure_instrucciones);

    free(buffer);
    free(inst_per_line);
    fclose(file);

    return inst_list;
}

int obtener_cantidad_segmentos(){
    return cfg_console->TAMANIO_LISTA_SEGMENTOS;
}

t_list* crear_lista_de_segmentos() {
    char** segmentos = cfg_console->SEGMENTOS;
    uint16_t* intSegmentos = calloc(cfg_console->TAMANIO_LISTA_SEGMENTOS,sizeof(uint8_t));
    t_list* seg_list = list_create();    //creo la lista de instrucciones
    for (int i = 0; i < cfg_console->TAMANIO_LISTA_SEGMENTOS; ++i) {
        intSegmentos[i] = atoi(segmentos[i]);
    }

    for (int i = 0; i < cfg_console->TAMANIO_LISTA_SEGMENTOS; ++i) {

        list_add(seg_list, &intSegmentos[i]);
    }
    //uint16_t * a = list_get(seg_list,1);
    return seg_list;
}

void cerrar_programa(){
    //matar_hilos();

    cortar_conexiones();


    log_info(logger_console,"TERMINADA_LA_CONFIG");
    log_info(logger_console,"TERMINANDO_EL_LOG");
    log_destroy(logger_console);

}
