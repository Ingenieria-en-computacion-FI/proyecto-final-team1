#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include "tokens.h"

/*
 * El parser reutiliza el Token del lexer (tokens.h).
 * TokenType ya está definido allí; no se redefine aquí.
 *
 * Para reconocer números y corchetes dentro del parser,
 * clasificar_lexema() devuelve TOKEN_INMEDIATO para números
 * y TOKEN_IDENTIFICADOR para registros. Los corchetes '['
 * y ']' llegan como TOKEN_DESCONOCIDO desde el lexer
 * porque el lexer los descarta como separadores; el parser
 * los maneja via el campo lexema directamente.
 */

/* ---- Tipos de operando ---- */
typedef enum {
    OP_NONE,
    OP_REG,   /* Registro (EAX, EBX…) */
    OP_IMM,   /* Inmediato (10, -5)    */
    OP_MEM    /* Memoria ([EAX+4])     */
} OperandType;

typedef struct {
    OperandType type;

    char    reg_name[8];
    int32_t imm_value;

    /* Modo memoria (SIB + desplazamiento) */
    char    base_reg[8];
    char    index_reg[8];
    int     scale;
    int32_t displacement;
    bool    has_base;
    bool    has_index;
} Operand;

/* ---- Instrucción ensamblada ---- */
typedef struct {
    char    mnemonic[10];
    Operand op1;
    Operand op2;
    int     operand_count;
} Instruction;

/* ---- Directiva de ensamblador ---- */
typedef struct {
    char name[16];
    char arguments[4][32];
    int  arg_count;
} Directive;

/* ---- Prototipos públicos ---- */
void        advance(Token** current);
bool        is_register(const char* str);
Operand     parse_operand(Token** current_token);
Operand     parse_memory_operand(Token** current_token);
Instruction parse_instruction(Token** current_token);
Directive   parse_directive(Token** current_token);

#endif /* PARSER_H */
