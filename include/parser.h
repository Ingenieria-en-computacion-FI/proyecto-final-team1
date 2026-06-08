#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>

// Supongamos que el Integrante 1 (Lexer) nos entrega este Token
typedef enum {
    TOK_IDENTIFIER, TOK_NUMBER, TOK_COMMA, 
    TOK_LBRACKET, TOK_RBRACKET, TOK_PLUS, TOK_STAR, TOK_NEWLINE, TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[32]; // Almacena el texto del token (ej. "MOV", "EAX", "10")
} Token;

// --- INTEGRANTE 2 ---

typedef enum {
    OP_NONE,
    OP_REG,      // Registro (EAX, EBX...)
    OP_IMM,      // Inmediato (10, 0x1A)
    OP_MEM       // Memoria ([EAX + EBX*2 + 4])
} OperandType;

typedef struct {
    OperandType type;
    
    // Para OP_REG u OP_IMM
    char reg_name[4]; 
    int32_t imm_value;
    
    // Para OP_MEM (Soporte SIB y Desplazamiento)
    char base_reg[4];
    char index_reg[4];
    int scale;           // Escalas permitidas: 1, 2, 4, 8
    int32_t displacement;
    bool has_base;
    bool has_index;
} Operand;

// Nodo principal para una línea de código ensamblador
typedef struct {
    char mnemonic[10];   // Ej: "MOV", "ADD", "JMP"
    Operand op1;
    Operand op2;
    int operand_count;
} Instruction;

// Firmas de las funciones del Parser
Instruction parse_instruction(Token** current_token);
Operand parse_operand(Token** current_token);
Operand parse_memory_operand(Token** current_token);

// Estructura para almacenar directivas (Ej: SECTION .data, DB 10)
typedef struct {
    char name[16];        // Ej: "SECTION", "DB"
    char arguments[4][32]; // Hasta 4 argumentos de texto
    int arg_count;
} Directive;

// Firma de las funciones restantes
Instruction parse_instruction(Token** current_token);
Directive parse_directive(Token** current_token);

#endif
