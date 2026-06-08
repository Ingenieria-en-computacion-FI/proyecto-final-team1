#ifndef TOKENS_H
#define TOKENS_H

// Definición del diccionario de tokens (Arquitectura IA-32)
typedef enum {
    REG_EAX = 250, REG_EBX, REG_ECX, REG_EDX,
    REG_ESI, REG_EDI, REG_EBP, REG_ESP,

    INST_MOV, INST_PUSH, INST_POP, INST_LEA, INST_MOVXZ,
    INST_ADD, INST_SUB, INST_INC, INST_DEC, INST_MUL, INST_DIV, INST_IMUL, INST_IDIV, INST_CMP, INST_NEG,
    INST_AND, INST_OR, INST_XOR, INST_NOT,
    INST_JMP, INST_JE, INST_JNE, INST_JG, INST_JGE, INST_JL, INST_JLE, INST_CALL, INST_RET, INST_NOP, INST_INT,

    DIR_DB, DIR_DW, DIR_DD, DIR_RESB, DIR_RESW, DIR_RESD,
    DIR_SECTION, DIR_GLOBAL, DIR_EXTERN, DIR_EQU,

    TOKEN_INMEDIATO,
    TOKEN_ETIQUETA,
    TOKEN_IDENTIFICADOR,
    TOKEN_EOF,
    TOKEN_DESCONOCIDO
} TokenType;

typedef struct {
    TokenType tipo;
    char lexema[64];
} Token;

// Prototipos
void iniciar_lexer(const char *filename);
Token obtener_siguiente_token();
void cerrar_lexer();

#endif
