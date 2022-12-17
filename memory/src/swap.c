#include <swap.h>

pthread_mutex_t mutex_archivo_swap;
pthread_mutex_t MUTEX_HUECO_DISPONIBLE;

/*Tengo un array de pagina_en_swap y a este array lo inicializo al crear la swap, despues cuando creo una pagina
 me fijo en la estructura cual no esta ocupado su posicion en swap*/
typedef struct{
    void* archivo;
    int fd;
    char* path_swap;
} archivo_swap;

typedef struct{
    uint32_t bit_ocupado;
    uint32_t posicion_swap;
} pagina_en_swap;

pagina_en_swap *paginas_en_swap;
archivo_swap* swap;
t_list* archivos;
void* memoria_usuario;
int cantidadPaginasEnSwap;

char * liberarPaginaEnSwapConPosicion(uint32_t posicion);

void liberarHuecoParaLaPaginaEnSwap(int i);

char* armarPath () {
    //char* ruta = malloc(strlen(cfg_memory->PATH_SWAP)+1);
    //TODO REVISAR QUE ONDA CON ESTO
    //string_append(&ruta, cfg_memory->PATH_SWAP); //Copia el string a la ruta
    return cfg_memory->PATH_SWAP;
}

bool crearSwap(){ // el swap se crea una sola vez
    int fd;
    char* path = armarPath();
    if((fd=open(path,O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR))==-1){
        char* a;
        if(errno == 17){
            remove(path);
            log_warning(logger_memory, "Habia un archivo de swap ya creado, se elimino para poder crear uno nuevo");
            fd=open(path,O_CREAT|O_EXCL|O_RDWR,S_IRUSR|S_IWUSR);
        }else{
            perror(&a);
            log_error(logger_memory,"Error al crear el archivo swap");
            exit(-1);
        }

    }
    log_debug(logger_memory,"Se creo el archivo swap");
    ftruncate(fd,cfg_memory->TAMANIO_SWAP); //El archivo es truncado al tamaño de swap

    swap = malloc(sizeof(archivo_swap));
    swap->path_swap = path;

    // hacer el mmap para que quede siempre abierto
    swap->archivo = mmap(NULL, cfg_memory->TAMANIO_SWAP,PROT_WRITE|PROT_READ, MAP_SHARED,fd,0);
    if (swap->archivo == MAP_FAILED)
    {
        perror("Error mapping the file");
    }
    close(fd);
    cantidadPaginasEnSwap = cfg_memory->TAMANIO_SWAP / cfg_memory->TAM_PAGINA;
    paginas_en_swap = calloc(cantidadPaginasEnSwap, sizeof(pagina_en_swap));
    for (int i = 0; i <cantidadPaginasEnSwap ; ++i) {
        paginas_en_swap[i].bit_ocupado = 0;
        paginas_en_swap[i].posicion_swap = i*cfg_memory->TAM_PAGINA;
    }
    return true;
}

void eliminarSwap() {
    if(munmap(swap->archivo,cfg_memory->TAMANIO_SWAP) == 0)  {
        // una vez que se escribio en swap y libero el espacio, ahi recien se hace el free del mmap
        //close(swap->fd); // cierro el archivo swap una vez que s etermino de liberar el archivo swap con el munmap
    }

    if(remove(swap->path_swap)==-1){
        log_error(logger_memory,"Error al borrar el archivo swap");
        exit(-1);
    } else {
        log_debug(logger_memory,"Se borro con exito el archivo swap");
    }
}

void* traerPaginaDeSwap(uint32_t posSwap){
    usleep(cfg_memory->RETARDO_SWAP * 1000);
    void* pagina = malloc(cfg_memory->TAM_PAGINA);
    pthread_mutex_lock(&mutex_archivo_swap);
    memcpy(pagina ,swap->archivo + posSwap, cfg_memory->TAM_PAGINA);
    pthread_mutex_unlock(&mutex_archivo_swap);
    return pagina;
}

void actualizarSwap(void* datos, uint32_t posicion_swap, uint32_t bit_modificado){
    if(bit_modificado == 1){
        usleep(cfg_memory->RETARDO_SWAP * 1000);
        pthread_mutex_lock(&mutex_archivo_swap);
        memset(swap->archivo + posicion_swap,"",cfg_memory->TAM_PAGINA);
        memcpy(swap->archivo + posicion_swap,datos,cfg_memory->TAM_PAGINA);
        pthread_mutex_unlock(&mutex_archivo_swap);
        log_info(logger_memory,"Se escribio pagina modificada en swap");
    }
    else if(bit_modificado == 0) {
        usleep(cfg_memory->RETARDO_SWAP * 1000);
        pthread_mutex_lock(&mutex_archivo_swap);
        memcpy(swap->archivo + posicion_swap, datos, cfg_memory->TAM_PAGINA);
        pthread_mutex_unlock(&mutex_archivo_swap);
        log_info(logger_memory, "Se escribio pagina en swap");

        //TODO: EN CASO DE QUE EN LA SWAP NO HAYA MAS ESPACIO COMO SE ACTUALIZA
    }
}
pagina_en_swap *buscarHuecoDisponible(){
    pthread_mutex_lock(&MUTEX_HUECO_DISPONIBLE);
    for (int i = 0; i <cantidadPaginasEnSwap ; ++i) {
        if(paginas_en_swap[i].bit_ocupado == 0){
            //log_debug(logger_memory,"Encontre espacio (hueco) en la swap para asignar: <%d>", i);
            pthread_mutex_unlock(&MUTEX_HUECO_DISPONIBLE);
            return &paginas_en_swap[i];
        }
    }
    log_error(logger_memory,"No hay espacio en la SWAP para otra pagina");
    pthread_mutex_unlock(&MUTEX_HUECO_DISPONIBLE);
}
uint32_t crearPaginaEnSwap(){
    pagina_en_swap* huecoDisponible = buscarHuecoDisponible();
    huecoDisponible->bit_ocupado = 1;
    return huecoDisponible->posicion_swap;
    //entro a mi estructura veo cual esta disponible su posicion y retorno esa posicion
}

char * liberarPagina(uint32_t posicion){
    //busco la posicion. pos/tam_pagina
    pthread_mutex_lock(&mutex_archivo_swap);
    memset(swap->archivo + posicion, 0, sizeof(cfg_memory->TAM_PAGINA));
    pthread_mutex_unlock(&mutex_archivo_swap);
    return liberarPaginaEnSwapConPosicion(posicion);
}

char * liberarPaginaEnSwapConPosicion(uint32_t posicion) {
    pthread_mutex_lock(&mutex_archivo_swap);
    int indice = posicion / cfg_memory->TAM_PAGINA;
    paginas_en_swap[indice].bit_ocupado = 0;
    /*
     *
    for (int i = 0; i <cantidadPaginasEnSwap ; ++i) {
        if(paginas_en_swap[i].posicion_swap == posicion){
            paginas_en_swap[i].bit_ocupado = 0;
            return;
        }

    }
     */
    pthread_mutex_unlock(&mutex_archivo_swap);
    return string_itoa(paginas_en_swap[indice].posicion_swap);
}




/*
bool modificado(marco){
    return marco->modificado;
}

t_list* marcosModificados(t_list* marcos){
    t_list* modificados = (t_list*)list_filter(marcos,modificado);
    return modificados;
}

void escribirPaginasModificadas(void* datos, uint32_t posicion_swap){

    t_list* paginasProcesadas = marcosModificados(paginasEnMemoria(pcb->id_proceso));

    asignarAlArchivo(pcb->id_proceso);
    log_info(memoria_logger,"Se asigno al archivo swap el archivo del pid %d \n|",pcb->id_proceso);
    for(int i=0;i<list_size(paginasProc);i++){
        pthread_mutex_lock(&mutex_contador_pid);
        contador_por_pid* contador  = (contador_por_pid*)list_get(contador_pid,pcb->id_proceso);
        contador->contadorAccesoSwap +=1 ;
        pthread_mutex_unlock(&mutex_contador_pid);
        t_p_2* pag=(t_p_2*)list_get(paginasProc,i);
        escribirPagEnSwap(pag);
        msync(archivo_swap,pcb->tamanio_proceso,MS_SYNC);
        cambiarMdePagina(pag->indice,pcb->id_proceso,0);
        log_info(memoria_logger,"Se escribio pagina modificada en swap \n");

        liberarMarco(pag->marco); // despues de escribir la pag, libero el marco de esa pagina

        log_info(memoria_logger,"Se libero el marco de la pagina modificada en swap \n");
        cambiarUdePagina(pag->indice,pcb->id_proceso,0);
        cambiarPdePagina(pag->indice,pcb->id_proceso,0);
        log_info(memoria_logger,"Se setean de la pag modificada los bits de p,u y m en 0 \n");
    }
}

void asignarAlArchivo(uint32_t pid) {
    bool archivos_con_pid(archivos_swap* un_archivo) {
        return un_archivo->pid == pid;
    }
    pthread_mutex_lock(&mutex_lista_archivo);
    archivos_swap* archivo = (archivos_swap*)list_find(archivos,archivos_con_pid);
    pthread_mutex_unlock(&mutex_lista_archivo);

    pthread_mutex_lock(&mutex_archivo_swap);
    archivo_swap = archivo->archivo;
    pthread_mutex_unlock(&mutex_archivo_swap);
}*/