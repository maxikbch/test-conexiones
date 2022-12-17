#include <main.h>

void handle_sigint(int sig) {
    cerrar_programa();
    exit(0);
}

char *path_config;


int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    path_config = argv[1];
    ip_cpu = argv[2];


    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar cpu");
        return EXIT_FAILURE;
    }
    //crearHilosServidores();

    if (!generar_conexiones()) {
        cerrar_programa();
        return EXIT_FAILURE;
    }

    cerrar_programa();

    return 0;
}



