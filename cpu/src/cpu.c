#include <cpu.h>


//si incluimos la config de memoria ya tenemos las entradasxtabla y tamaño pagina, que se carga en el init cpu 
//si esta la variable en init.h , inicializada en init.c y la uso aca llega? para usar tiempocpu y la otra

//DATOS DE MEMORIA  ->VER COMO LLEGAN
uint32_t entradasPorTabla;
uint32_t tamanioDePagina;

bool tengoQueEsperarRespuestaContinuar = false;


//TLB
t_queue* tlb; //hacer el free
instr_t* instruccion;
char* nombre_instruccion_actual;
int marco;

//REGISTROS
uint32_t registroAX = 0;
uint32_t registroBX = 0;
uint32_t registroCX = 0;
uint32_t registroDX = 0;

//Variables actuales
int segmento_actual;
int pagina_actual;
int indiceTabla_actual;
int dir_fisica_actual;
int dir_logica_actual;
int desplazamiento_seg_actual;




//INICIAR CPU: 
//              CONFIG -> cfg_cpu
//              LOGGER -> logger_cpu
//INICIAR CONEXION CON MEMORIA -> fd_memoria
//INICIAR CONEXION DISPATCH -> fd_dispatch
//INICIAR CONEXION INTERRUPT -> fd_interrupt
//----
//FINALIZAR CPU
//EXIT SUCCESS



//CICLO DE INSTRUCCIONES
void ciclo_instrucciones(){

    while(!tengoQueEsperarRespuestaContinuar){ //pcb_actual != NULL -> deberia usarse un booleano para cortar esto

        instruccion = fetch(pcb_actual);

        nombre_instruccion_actual = decode(instruccion);

        execute();
    }
    tengoQueEsperarRespuestaContinuar = false;
}

//TODO reiniciar_registros(); VER DESPUES DE QUE INSTRUCCION HACERLO, interrupt, page fault, bloqueo, io, etc

//FETCH
//TODO CONDICION DE CARRERA CUANDO DEBUGEO EN IO -> LIBERAR PCB (devolverpcb)
instr_t* fetch(pcb* pcbActual){
    return list_get(pcbActual->instr, pcbActual->programCounter);
}

//DECODE
char* decode(instr_t* instruccion){
    return instruccion->id;
}

//EXECUTE
void execute(){ //instruccion: SET ADD MOVIN MOVOUT IO EXIT



    if(strcmp(nombre_instruccion_actual, "SET") == 0){
        //SET (Registro, Valor)
        log_info(logger_cpu, "PID: <%d> - Ejecutando <SET> - <%s> - <%s> ", pcb_actual->id, instruccion->param1, instruccion->param2);

        int valor = atoi(instruccion->param2);
        escribir_en_registro(instruccion->param1, valor);

        simularRetardo("Tiempo estimado <SET> %d ", "", logger_cpu, cfg_cpu->RETARDO_INSTRUCCION);

        pcb_actual->programCounter++;


        if (check_interrupt()){
            log_info(logger_cpu, "Se desaloja el Proceso <%d> por Interrupcion", pcb_actual->id);
            interrupcion = 0;
            sem_post(&sem_interrupt); //Reanudacion hilo interrupt
            tengoQueEsperarRespuestaContinuar = true;
            devolverPcb(PROCESO_DESALOJADO, fd_kernel);
        }
    }else if(strcmp(nombre_instruccion_actual, "ADD") == 0){
        //ADD (Registro Destino, Registro Origen)
        log_info(logger_cpu, "PID: <%d> - Ejecutando <ADD> - <%s> - <%s> ", pcb_actual->id, instruccion->param1, instruccion->param2);

        uint32_t valor_a_escribir = leer_de_registro(instruccion->param2);
        escribir_en_registro(instruccion->param1, valor_a_escribir);

        simularRetardo("Tiempo estimado <ADD> %d ", "", logger_cpu, cfg_cpu->RETARDO_INSTRUCCION);

        pcb_actual->programCounter++;

        if (check_interrupt()){
            log_info(logger_cpu, "Se desaloja el Proceso <%d> por Interrupcion", pcb_actual->id);
            interrupcion = 0;
            sem_post(&sem_interrupt); //Reanudacion hilo interrupt
            tengoQueEsperarRespuestaContinuar = true;
            devolverPcb(PROCESO_DESALOJADO, fd_kernel);
        }

    }else if(strcmp(nombre_instruccion_actual, "MOV_IN") == 0){
        //MOV_IN (Registro, Dirección Lógica)
        log_info(logger_cpu, "PID: <%d> - Ejecutando <MOV_IN> - <%s> - <%s> ", pcb_actual->id, instruccion->param1, instruccion->param2);

        dir_logica_actual = direccion_logica();
        
        buscar_marco();

    }else if(strcmp(nombre_instruccion_actual, "MOV_OUT") == 0){
        //MOV_OUT (Dirección Lógica, Registro)
        log_info(logger_cpu, "PID: <%d> - Ejecutando <MOV_OUT> - <%s> - <%s> ", pcb_actual->id, instruccion->param1, instruccion->param2);

        dir_logica_actual = direccion_logica();

        buscar_marco();

    }else if(strcmp(nombre_instruccion_actual, "I/O") == 0){
        //I/O (Dispositivo, Registro / Unidades de trabajo)

        //Tipos de retorno a Kernel: BLOCKED_TECLADO, BLOCKED_PANTALLA, BLOCKED_IO

        log_info(logger_cpu, "PID: <%d> - Ejecutando <I/O> - <%s> - <%s> ", pcb_actual->id, instruccion->param1, instruccion->param2);

        char* dispositivo = instruccion->param1;
        
        if(strcmp(dispositivo, "PANTALLA") == 0){
            tengoQueEsperarRespuestaContinuar = true;
            log_info(logger_cpu,"Desalojando - PID: <%d> por I/O <%s> ",pcb_actual->id, dispositivo);
            pcb_actual->programCounter++;
            devolverPcb(BLOCKED_PANTALLA, fd_kernel);
        }else if (strcmp(dispositivo, "TECLADO") == 0){
            tengoQueEsperarRespuestaContinuar = true;
            log_info(logger_cpu,"Desalojando - PID: <%d> por I/O <%s> ",pcb_actual->id, dispositivo);
            pcb_actual->programCounter++;
            devolverPcb(BLOCKED_TECLADO, fd_kernel);
        }else{
            tengoQueEsperarRespuestaContinuar = true;
            log_info(logger_cpu,"Desalojando - PID: <%d> por I/O <%s> ",pcb_actual->id, dispositivo);
            pcb_actual->programCounter++;
            devolverPcb(BLOCKED_IO, fd_kernel);
        }
                    
        //Kernel debe acceder a la pcb en la instruccion en la que quedo, acceder a la instruccion y a los campos para obtener lo que necesita: DISPOSITIVO Y REGISTRO/UNIDADESTRABAJO


    }else if(strcmp(nombre_instruccion_actual, "EXIT") == 0){
        //EXIT
        uint32_t pid = pcb_actual->id;
        tengoQueEsperarRespuestaContinuar = true;
        log_info(logger_cpu, "PID: <%d> - <EXIT> ", pid);
        devolverPcb(PROCESO_TERMINADO, fd_kernel); //Tambien actualiza registros y reinicia valores

        if (!tlb_sin_entradas()) {
            limpiar_proceso_tlb(pid); //Saco el proceso de la tlb
        }
    }
   //RETURN?

}

//CHECK_INTEERUPT
bool check_interrupt(){
    return interrupcion == 1;
}


//AUXILIARES

void escribir_en_registro(char* registro, uint32_t valor){
    if(strcmp(registro, "AX") == 0){
        registroAX = valor;
    }else if(strcmp(registro, "BX") == 0){
        registroBX = valor;
    }else if(strcmp(registro, "CX") == 0){
        registroCX = valor;
    }else if(strcmp(registro, "DX") == 0){
        registroDX = valor;
    }
    log_info(logger_cpu, "Se escribio correctamente el valor <%d> en el registro <%s>", valor, registro);

}

uint32_t leer_de_registro(char* registro){
    uint32_t valor_registro = -1;

    if(strcmp(registro, "AX") == 0){
        valor_registro = registroAX;
    }else if(strcmp(registro, "BX") == 0){
        valor_registro = registroBX;
    }else if(strcmp(registro, "CX") == 0){
        valor_registro = registroCX;
    }else if(strcmp(registro, "DX") == 0){
        valor_registro = registroDX;
    }

    if(valor_registro >= 0) {
        log_info(logger_cpu, "Se leyo correctamente el valor <%d> del registro <%s>", valor_registro, registro);
    }else{
        log_info(logger_cpu, "El registro <%s> no tenia información ");
    }
    return valor_registro;
}





void devolverPcb(op_code codigo, uint32_t accepted_fd){
	//Copiamos los valores de los registros antes de devolver la pcb
    pcb_actual->registrosCpu->AX = registroAX;
    pcb_actual->registrosCpu->BX = registroBX;
    pcb_actual->registrosCpu->CX = registroCX;
    pcb_actual->registrosCpu->DX = registroDX;

    enviar_paquete_pcb(pcb_actual, accepted_fd, codigo, logger_cpu);

	liberarPcb(pcb_actual);

    //REINICIAR REGISTROS, INSTRUCCION, MARCO, SEGACT PAGACT INDTABLAACT DF DL
    pcb_actual=NULL;
    reiniciar_registros();
    reiniciar_valores();
    //CPU VACIA ->KERNEL
}


int direccion_logica(){
    int dl;
    if(strcmp(nombre_instruccion_actual, "MOV_IN") == 0){
        dl = atoi(instruccion->param2);
    }else if(strcmp(nombre_instruccion_actual, "MOV_OUT") == 0){
        dl = atoi(instruccion->param1);
    }
    return dl;
}

int direccion_fisica(int marco){
    int desplazamiento_pag = desplazamiento_pagina(desplazamiento_seg_actual);

    return marco + desplazamiento_pag;
}





//MMU
//tam_max_segmento = cant_entradas_por_tabla * tam_pagina	
//num_segmento = floor(dir_logica / tam_max_segmento)	
//desplazamiento_segmento = dir_logica % tam_max_segmento	
//num_pagina = floor(desplazamiento_segmento / tam_pagina)	
//desplazamiento_pagina = desplazamiento_segmento % tam_pagina	

//DL= SEG | PAG | DESP. PAG. -> -> Viene en el archivo del proceso (MOV_IN(R, DL))  y es un string que hay que parsear a  int
//DF= MARCO | DESP. PAG.


//SOLO USADA POR MOVIN o MOVOUT
//BUSCAR EL MARCO, PUEDE TERMINAR EN 
//1) SEGMENTATION FAULT -> Desaloja y devuelve pcb a kernel
//2) TLB HIT -> ESCRIBE/LEE 
//3) TLB MISS 
//  3.1) ACCESO_RESULTO_PAGE_FAULT -> Devuleve pcb a kernel
//  3.2) ERROR_INDICE_TP -> Que hace?
//  3.3) ENVIO_MARCO_CORRESPONDIENTE -> Recibe el marco
//      3.3.1) MOV_IN -> Ejecuta lectura -> LECTURA_REALIZADA ->Recibe el valor y lo guarda en el registro
//      3.3.2) MOV_OUT -> Ejecuta Escritura -> ESCRITURA_REALIZADA ->Finaliza

void buscar_marco(){

    //VER SEGEMENTATION FAULT
    segmento_actual = numero_segmento(dir_logica_actual);
    desplazamiento_seg_actual = desplazamiento_segmento(dir_logica_actual);
    int tam_seg_actual = tamanio_segmento_proceso(pcb_actual, segmento_actual);

    if (desplazamiento_seg_actual >= tam_seg_actual){
        log_error(logger_cpu, "SEGMENTATION FAULT en el proceso, el desplazamiento supera el maximo: PID: <%d> - SEGMENTO: <%d> - DESPLAZAMIENTO: >%d> - SUPERA MAXIMO: <%d> ",
                  pcb_actual->id, segmento_actual, desplazamiento_seg_actual, tam_seg_actual);
        //pcb_actual->programCounter -= 1; //No termine MOVIN o MOVOUT por ende en la prox iteracion estará en TLB o en memoria, no actualiazo el PC
        devolverPcb(ERROR_SIGSEGV, fd_kernel);
        tengoQueEsperarRespuestaContinuar = true;
        return;
    }

    pagina_actual = numero_pagina(desplazamiento_seg_actual);
    segmento* tab_seg = list_get(pcb_actual->tablaSegmentos, segmento_actual);
    indiceTabla_actual = tab_seg->indiceTablaPaginas;

    //CONSULTA EN TLB PRIMERO
    int respuesta_tlb;
    if(tlb_sin_entradas()){
        respuesta_tlb = -1;
    }else {
        respuesta_tlb = consultar_tlb(pcb_actual->id, segmento_actual, pagina_actual);
    }

    if (respuesta_tlb < 0) { //TLB MISS o NO HAY TLB
        if (!tlb_sin_entradas()) {
            log_info(logger_cpu, "PID: <%d> - TLB MISS - SEGMENTO: <%d> - PAGINA: <%d> ",
                 pcb_actual->id, segmento_actual, pagina_actual);
        }

        if(tlb_sin_entradas()){
            log_info(logger_cpu, "La TLB tiene no tiene capacidad para agregar entradas, se busca marco en Memoria ");
        }

        log_info(logger_cpu, "Solicitud Marco a Memoria: PID: <%d> - PAGINA: <%d> - INDICE TABLA PAGINA: <%d> ",
                 pcb_actual->id, pagina_actual, indiceTabla_actual);

        //PRIMER ACCESO A MEMORIA PARA MARCO
        //Envio codigo, pagina e indiceTP a Memoria
        op_code codigo = SOLICITUD_MARCO;
        uint32_t array_datos[] = {3, pcb_actual->id, indiceTabla_actual, pagina_actual};
        //Enviar array de ints
        tengoQueEsperarRespuestaContinuar = true;
        enviar_int_array(array_datos, fd_memoria, codigo, logger_cpu);
        return;
        //Recibo MARCO o PF EN COMUNICACION.C, y ejecuto un método o el otro

    }else{ //TLB HIT
        log_info(logger_cpu, "PID: <%d> - TLB HIT - SEGMENTO: <%d> - PAGINA: <%d> ",
                 pcb_actual->id, segmento_actual, pagina_actual);

        marco = respuesta_tlb;

        dir_fisica_actual = direccion_fisica(marco);

        if (strcmp(instruccion->id, "MOV_IN") == 0){
        ejecutar_lectura();
        }else if(strcmp(instruccion->id, "MOV_OUT") == 0){
        ejecutar_escritura();
        }
    }


}

void page_fault(){
    //En caso de PF devuelvo la pcb en la estructra pcb_page_fault
    //Con: segmento_actual y pagina_actual
    log_info(logger_cpu, "Page Fault: Se devuelve la pcb a Kernel - PID: <%d> - SEGMENTO: <%d> - PAGINA: <%d> ", pcb_actual->id, segmento_actual, pagina_actual);

    pcb_page_fault* pcbPF = malloc(sizeof(pcb_page_fault));

    pcbPF->segmento = segmento_actual;
    pcbPF->pagina = pagina_actual;
    pcbPF->pcb = pcb_actual;

    if (check_interrupt()){
        log_info(logger_cpu, "Se desaloja el Proceso <%d> por Interrupcion y PageFault", pcbPF->pcb->id);
        interrupcion = 0;
        sem_post(&sem_interrupt); //Reanudacion hilo interrupt
        //devolverPcbPF(PROCESO_DESALOJADO_CON_PAGEFAULT, fd_kernel);
    }

    devolverPcbPF(pcbPF, BLOCKED_PF, fd_kernel);

}

void devolverPcbPF(pcb_page_fault* pcbPF, op_code codigo, uint32_t accepted_fd){
	//Copiamos los valores de los registros antes de devolver la pcb
    pcb* pcbAPasar = pcbPF->pcb;
    pcbAPasar->registrosCpu->AX = registroAX;
    pcbAPasar->registrosCpu->BX = registroBX;
    pcbAPasar->registrosCpu->CX = registroCX;
    pcbAPasar->registrosCpu->DX = registroDX;

    enviar_paquete_pcbPf(pcbPF, accepted_fd, codigo, logger_cpu);

	liberarPcb(pcb_actual);
    free(pcbPF);

    //REINICIAR REGISTROS, INSTRUCCION, MARCO, SEGACT PAGACT INDTABLAACT DF DL
    pcb_actual=NULL;
    reiniciar_registros();
    reiniciar_valores();

}

void recibir_marco_memoria(){
    marco = recibirValor_uint32(fd_memoria, logger_cpu);
    log_info(logger_cpu,"Se recibio el marco solicitado a Memoria: <%d>", marco);

    if(!tlb_sin_entradas()) {
        agregar_a_tlb(pcb_actual->id, segmento_actual, pagina_actual, marco);
    }

    dir_fisica_actual = direccion_fisica(marco);

    if (strcmp(instruccion->id, "MOV_IN") == 0){
        ejecutar_lectura();
    }else if(strcmp(instruccion->id, "MOV_OUT") == 0){
        ejecutar_escritura();
    }else{
        log_error(logger_cpu,"Instruccion no reconocida, no es ni MOV_IN ni MOV_OUT");
    }


}


//MOV_IN
void ejecutar_lectura(){

    int array_datos[] = {3, pcb_actual->id, dir_fisica_actual, marco};

    log_info(logger_cpu, "Lectura de Memoria: pido el valor de la Direccion Fisica: <%d> ", dir_fisica_actual);
    tengoQueEsperarRespuestaContinuar = true;
    enviar_int_array(array_datos, fd_memoria, PEDIDO_LECTURA, logger_cpu);
}

//TERMINA MOV_IN
void terminar_ejecucion_lectura(uint32_t leido){ //TERMINA EL MOV_IN ESCRIBIENDO EN EL REGISTRO ASOCIADO
    log_info(logger_cpu, "Recibo de memoria el valor: <%d> y lo escribo en el registro: <%s> ", leido, instruccion->param1);

    escribir_en_registro(instruccion->param1, leido);
    //TODO //LOGGER TERMINE INST MOVIN
    pcb_actual->programCounter++;

    if (check_interrupt()){
        log_info(logger_cpu, "Se desaloja el Proceso <%d> por Interrupcion", pcb_actual->id);
        interrupcion = 0;
        sem_post(&sem_interrupt); //Reanudacion hilo interrupt
        devolverPcb(PROCESO_DESALOJADO, fd_kernel);
    }else{
        //TERMINE MOVIN
        tengoQueEsperarRespuestaContinuar = false;
        ciclo_instrucciones();
    }
}


//MOV_OUT
void ejecutar_escritura(){

    int valor = leer_de_registro(instruccion->param2); //REGISTRO ASOCIADO AL MOV_OUT

    int array_datos[] = {4, pcb_actual->id, dir_fisica_actual, valor, marco};

    log_info(logger_cpu, "Escritura de Memoria: envio el valor: <%d> en la Direccion Fisica: <%d> ", valor, dir_fisica_actual);

    tengoQueEsperarRespuestaContinuar = true;
    enviar_int_array(array_datos, fd_memoria, PEDIDO_ESCRITURA, logger_cpu);
}

//TERMINA MOV_OUT
void terminar_ejecucion_escritura(){
    //VER SI SE COPIO
    pcb_actual->programCounter++;
    if (check_interrupt()){
        log_info(logger_cpu, "Se desaloja el Proceso <%d> por Interrupcion", pcb_actual->id);
        interrupcion = 0;
        sem_post(&sem_interrupt); //Reanudacion hilo interrupt
        devolverPcb(PROCESO_DESALOJADO, fd_kernel);
    }else{
        //LOGGER TERMINE MOV_OUT
        tengoQueEsperarRespuestaContinuar = false;
        ciclo_instrucciones();
    }
}





//CALCULOS MMU

int tamanio_maximo_segmento(){
    return entradasPorTabla * tamanioDePagina; //VER SI VA O SI NO RECIBIRLAS DESDE MEMORIA
}

int numero_segmento(int dir_logica){
    int maximo = tamanio_maximo_segmento();

    return floor(dir_logica / maximo);
}

int desplazamiento_segmento(int dir_logica){
    int maximo = tamanio_maximo_segmento();

    return dir_logica % maximo;
}

int numero_pagina(int desplazamiento_segmento){
    return floor(desplazamiento_segmento / tamanioDePagina);
}

int desplazamiento_pagina(int desplazamiento_segmento){
    return desplazamiento_segmento / tamanioDePagina;
}

int cantidad_segmentos_proceso(pcb* pcb){
    return list_size(pcb->tablaSegmentos);
}

int tamanio_segmento_proceso(pcb* pcb, int nsegmento){
    segmento* seg = list_get(pcb->tablaSegmentos, nsegmento);
    return seg->tamanioSegmento; //Accede a la lista de segmentos, busca ese y accede al campo tamaño
}



//REINICIO DE ESTRUCTURAS AL DESALOJAR/TERMINAR UN PROCESO

void reiniciar_registros(){
    registroAX = 0;
    registroBX = 0;
    registroCX = 0;
    registroDX = 0;
}

void reiniciar_valores(){
    instruccion = NULL;
    marco = -1;
    segmento_actual = -1;
    pagina_actual = -1;
    indiceTabla_actual = -1;
    dir_fisica_actual = -1;
    dir_logica_actual = -1;
    nombre_instruccion_actual = NULL;
    desplazamiento_seg_actual = -1;
}



//TLB

void iniciar_tlb(){
	tlb = queue_create(); //free(tlb) en init
}

bool tlb_sin_entradas(){
    return (cfg_cpu->ENTRADAS_TLB == 0);
}

void limpiar_proceso_tlb(uint32_t id){
	//Busca registros con el pid id y borrarlos de la cola
	bool entrada_equivalente(entrada_tlb* e){
        return e->epid == id;
    }
    list_remove_and_destroy_all_by_condition(tlb->elements, (void*) entrada_equivalente, (void*)free);
    log_info(logger_cpu, "Se borraron de la TLB los registros del proceso PID: <%d> ", id);
}

bool tlb_completa(){
    return queue_size(tlb) == cfg_cpu->ENTRADAS_TLB;
}

void agregar_a_tlb(int idp, uint32_t nsegmento, uint32_t npagina, uint32_t nmarco){
    //Creo la entrada
    entrada_tlb* agregar = malloc(sizeof(entrada_tlb));
    agregar -> epid = idp;
    agregar -> esegmento = nsegmento;
    agregar -> epagina = npagina;
    agregar -> emarco = nmarco;

    //Asigno referencias de tiempo
    struct timeval tiempo;
    gettimeofday(&tiempo, NULL);
    double tiempoActual = tiempo.tv_sec -tiempoInicialCpu;
    agregar -> instanteCarga = tiempoActual;
    agregar ->ultimaReferencia = tiempoActual;


    if(tlb_completa()){
        log_info(logger_cpu, "TLB completa, se reemplazará una entrada");

        reemplazar_entrada(agregar);
    }else {
        log_info(logger_cpu, "TLB con entradas libres, se agrega directamente");

        queue_push(tlb, agregar);
    }

    //Mostramos el estado de la tlb
    mostrar_tlb();

}

int consultar_tlb(int id, int seg, int pag){
    int marco_buscado = -1;

    bool buscar(entrada_tlb* entrada){
        return entrada->epid==id && entrada->esegmento==seg && entrada->epagina==pag;
    }

    entrada_tlb* buscada = list_find(tlb->elements, (void*) buscar);

    if (buscada != NULL){
        //Actualizo ultima referencia de la entrada
        struct timeval t;
		gettimeofday(&t, NULL);
		double refActual  = t.tv_sec - tiempoInicialCpu;
        buscada->ultimaReferencia = refActual;

        marco_buscado = buscada->emarco;
    }

    return marco_buscado;
}

void reemplazar_entrada(entrada_tlb* entrada){
    if (strcmp(cfg_cpu->REEMPLAZO_TLB , "FIFO") == 0){
        log_info(logger_cpu, "Algoritmo de Reemplazo FIFO");
        algoritmo_fifo(entrada);
    }

    if (strcmp(cfg_cpu->REEMPLAZO_TLB , "LRU") == 0){
        log_info(logger_cpu, "Algoritmo de Reemplazo LRU");
        algoritmo_lru(entrada);
    }
}

void algoritmo_fifo(entrada_tlb* entrada){
    //Ordeno la TLB según el menor instante de carga y reemplazo la primer pagina, que será la que tiene fué cargada primero

    list_sort(tlb->elements, (void*) menor_instante_carga);

    entrada_tlb* primera = queue_pop(tlb);

    log_info(logger_cpu, "Se saca de TLB el Proceso de PID: <%d> - SEGMENTO: <%d> - PAGINA: <%d> - MARCO: <%d> - ULT REF: <%f> - CARGA: <%f> ",
            primera->epid, primera->esegmento, primera->epagina, primera->emarco, primera->ultimaReferencia, primera->instanteCarga);


    //Agrego la nueva
    queue_push(tlb, entrada);
    log_info(logger_cpu, "Se agregó a TLB el Proceso de PID: <%d> - SEGMENTO: <%d> - PAGINA: <%d> - MARCO: <%d> - ULT REF: <%f> - CARGA: <%f> ",
             primera->epid, primera->esegmento, primera->epagina, primera->emarco, primera->ultimaReferencia, primera->instanteCarga);

    free(primera);
}

void algoritmo_lru(entrada_tlb* entrada){
    //Ordeno la TLB según la menor ultima referencia que tuvo la pagina de un segmento, reemplazo la primera pagina que será la menos usada en ese instante
    list_sort(tlb->elements, (void*) menor_ultima_referencia);
    entrada_tlb* primera = queue_pop(tlb);

    log_info(logger_cpu, "Se saca de TLB el Proceso de PID: <%d> - SEGMENTO: <%d> - PAGINA: <%d> - MARCO: <%d> - ULT REF: <%f> - CARGA: <%f> ",
             primera->epid, primera->esegmento, primera->epagina, primera->emarco, primera->ultimaReferencia, primera->instanteCarga);


    //Agrego la nueva
    queue_push(tlb, entrada);
    log_info(logger_cpu, "Se agregó a TLB el Proceso de PID: <%d> - SEGMENTO: <%d> - PAGINA: <%d> - MARCO: <%d> - ULT REF: <%f> - CARGA: <%f> ",
             primera->epid, primera->esegmento, primera->epagina, primera->emarco, primera->ultimaReferencia, primera->instanteCarga);

    //free(primera);
}

bool menor_instante_carga(entrada_tlb* e1, entrada_tlb* e2) {
    return e1->instanteCarga <= e2->instanteCarga;
}

bool menor_ultima_referencia(entrada_tlb* e1, entrada_tlb* e2) {
    return e1->ultimaReferencia <= e2->ultimaReferencia;
}

void limpiar_tlb(){
    queue_destroy_and_destroy_elements(tlb,(void*)free);
}

void mostrar_tlb(){
    log_info(logger_cpu, "ESTADO TLB: ");
    //Ordenamos por instante de carga
    list_sort(tlb->elements, (void*) menor_instante_carga);
    //Mostramos cada entrada
    list_iterate(tlb->elements, (void*) iterador_tlb);
}

void iterador_tlb(entrada_tlb* e){
    //Logger por entrada
    log_info(logger_cpu, "Entrada TLB: | PID: <%d> | SEGMENTO: <%d> | PAGINA: <%d> | MARCO: <%d> | ULT REF: <%f> | CARGA: <%f> ",
             e->epid, e->esegmento, e->epagina, e->emarco, e->ultimaReferencia, e->instanteCarga);
}


