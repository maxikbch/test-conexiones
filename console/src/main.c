#include <main.h>
t_proceso* proceso;

bool mostrarInstruccionesLog = true;
bool controlarPuntoEntrada = true;
bool mostrarLiberacionProceso = false;
int main(int argc, char *argv[])
{
    signal(SIGINT, handle_sigint);
    path_config = argv[1];
    init(path_config);

    if(argumentosInvalidos(argc,argv)){
        //ERROR
        printf("error");
        return 0;
    }
    cargar_configuracion(path_config);

    if (!generar_conexiones()){
        //cerrar_programa(logger_console,config);
        return EXIT_FAILURE;
    }
    if(controlarPuntoEntrada){
        int a;
        printf("Inserte cualquier numero para continuar: \n");
        scanf("%d", &a);
    }


    proceso = malloc(sizeof(t_proceso));
    proceso->instrucciones = crear_lista_de_instrucciones(path_pseudo);
    proceso->cantidad_segmentos = obtener_cantidad_segmentos();
    proceso->tam_segmentos = crear_lista_de_segmentos();

    if(mostrarInstruccionesLog){
        list_iterate(proceso->instrucciones,closure_mostrarListaInstrucciones);
    }

    paquete(proceso, fd_kernel);

    esperarOrdenes();
    liberarProceso();
    cerrar_programa();


    //cerrar_programa(logger_console,file_cfg_console);
    return EXIT_SUCCESS;
}
bool liberarProceso(){

    if(mostrarLiberacionProceso){
        log_info(logger_console,"LIBERANDO INSTRUCCIONES");
        free(proceso->instrucciones);
        log_info(logger_console,"LIBERANDO TAMANIO DE LOS SEGMENTOS");
        free(proceso->tam_segmentos);
        log_info(logger_console,"LIBERANDO PROCESO EN SU TOTALIDAD");
        free(proceso);
    }else{
        free(proceso->instrucciones);
        free(proceso->tam_segmentos);
        free(proceso);
    }

    /*
     * if (fallo)
     */
    return true;
}
void handle_sigint(int sig){
    //habria que meter una funcion que mate los hilos si hay
    cerrar_programa();
    exit(0);
}
