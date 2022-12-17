#ifndef MEMORY_PAGINAS_H
#define MEMORY_PAGINAS_H
#define cantidad_tablas 100
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <utils/loggers_configs.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <planificacion.h>
#include <swap.h>
#include <auxiliar.h>
extern pthread_mutex_t MUTEX_ID;
extern t_list* procesos;
extern tabla_de_paginas* tablas_de_pagina;



int* inicializar_estructuras_proceso(int* array);
bool iniciar_lista_de_tabla_de_paginas();
char * iniciar_pagina(entrada_pagina* pagina);
int *buscar_tablas_disponibles(int pid, int cantidad_necesaria);
uint32_t buscarMarcoAsociado(uint32_t indiceTablaDePaginas, uint32_t indicePagina);
entrada_pagina* buscarEntrada(uint32_t indiceTablaDePaginas, uint32_t indicePagina);
void liberar_estructuras_proceso(int pid);
void crear_paginas_necesarias(tabla_de_paginas* tabla);


#endif //MEMORY_PAGINAS_H