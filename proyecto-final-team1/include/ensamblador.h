#ifndef ENSAMBLADOR_H
#define ENSAMBLADOR_H

#include <stdint.h>
#include "tokens.h"

void ejecutar_primera_pasada(Token* token_list);
void ejecutar_segunda_pasada(Token* token_list,
                              uint8_t* binario_final,
                              uint32_t* tamanio_total);

#endif
