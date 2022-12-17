//
// Created by utnso on 12/9/22.
//

#ifndef MEMORY_AUXILIAR_H
#define MEMORY_AUXILIAR_H
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    uint32_t marco;
    u_int8_t bit_presencia;
    u_int8_t bit_uso;
    u_int8_t bit_modificado;
    uint32_t pos_en_swap;
} entrada_pagina;

typedef struct {
    u_int8_t ocupado;
    entrada_pagina* paginas;
} tabla_de_paginas;

#endif //MEMORY_AUXILIAR_H