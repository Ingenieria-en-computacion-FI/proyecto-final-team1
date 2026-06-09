#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "parser.h"

uint32_t codificar_instruccion_ia32(
    Instruction* inst,
    uint8_t* output_buffer
);

#endif