#include <main.h>

void handle_sigint(int sig){
    cerrar_programa();
    exit(0);
}

char* path_config;

void testAlgorimosClock(){
    //t_process_memory* procesoTest = malloc(sizeof(t_process_memory));

}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    path_config = argv[1];
    ip_memory= argv[2];

    //testAlgorimosClock();

    if(!init(path_config) || !cargar_configuracion(path_config) || !init_memoria()){
        cerrar_programa();
        printf("No se pudo inicializar memoria");
        return EXIT_FAILURE;
    }

    if (!generar_conexiones()){
        cerrar_programa();
        return EXIT_FAILURE;
    }

cerrar_programa();

    return 0;
}


