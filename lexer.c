#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"

typedef struct {
    const char *palabra;
    TokenType tipo;
} DiccionarioToken;

static const DiccionarioToken tabla_busqueda[] = {
    {"EAX", REG_EAX}, {"EBX", REG_EBX}, {"ECX", REG_ECX}, {"EDX", REG_EDX},
    {"ESI", REG_ESI}, {"EDI", REG_EDI}, {"EBP", REG_EBP}, {"ESP", REG_ESP},
    {"MOV", INST_MOV}, {"PUSH", INST_PUSH}, {"POP", INST_POP}, {"LEA", INST_LEA},
    {"ADD", INST_ADD}, {"SUB", INST_SUB}, {"INC", INST_INC}, {"DEC", INST_DEC}, 
    {"JMP", INST_JMP}, {"JE", INST_JE}, {"JNE", INST_JNE}, {"NOP", INST_NOP}, {"INT", INST_INT},
    {"DB", DIR_DB}, {"DW", DIR_DW}, {"DD", DIR_DD}, 
    {"SECTION", DIR_SECTION}, {"GLOBAL", DIR_GLOBAL}, {"EXTERN", DIR_EXTERN}
};

static const int num_palabras = sizeof(tabla_busqueda) / sizeof(tabla_busqueda[0]);

static FILE *archivo_fuente = NULL;
static char linea_actual[256];
static char *token_actual = NULL;

void iniciar_lexer(const char *filename) {
    archivo_fuente = fopen(filename, "r");
    if (archivo_fuente == NULL) {
        printf("Error: No se pudo abrir el archivo fuente: %s\n", filename);
    }
}

void cerrar_lexer() {
    if (archivo_fuente != NULL) {
        fclose(archivo_fuente);
    }
}

void estandarizar_token(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

TokenType clasificar_lexema(const char *lexema) {
    int len = strlen(lexema);
    if (len > 0 && lexema[len - 1] == ':') return TOKEN_ETIQUETA;

    // Busca en la tabla de instrucciones, registros y directivas
    for (int i = 0; i < num_palabras; i++) {
        if (strcmp(lexema, tabla_busqueda[i].palabra) == 0) {
            return tabla_busqueda[i].tipo; 
        }
    }
    
    // Si empieza con comillas, es una cadena de texto (lo tratamos como inmediato)
    if (lexema[0] == '\'' || lexema[0] == '"') {
        return TOKEN_INMEDIATO;
    }

    // Si es un número (positivo o negativo)
    if (isdigit(lexema[0]) || (lexema[0] == '-' && isdigit(lexema[1]))) {
        return TOKEN_INMEDIATO;
    }
    
    // Si empieza con letra, guion bajo, o un PUNTO (para que pase .DATA y .TEXT)
    if (isalpha(lexema[0]) || lexema[0] == '_' || lexema[0] == '.') {
        return TOKEN_IDENTIFICADOR;
    }
    
    // MANEJO BÁSICO DE ERRORES LÉXICOS
    printf(">> [ADVERTENCIA LEXICA] Simbolo no reconocido: '%s'\n", lexema);
    return TOKEN_DESCONOCIDO; 
}

Token obtener_siguiente_token() {
    Token t;
    t.tipo = TOKEN_EOF;
    t.lexema[0] = '\0';

    if (archivo_fuente == NULL) return t;

    while (token_actual == NULL) {
        if (fgets(linea_actual, sizeof(linea_actual), archivo_fuente) == NULL) return t;

        char *comentario = strchr(linea_actual, ';');
        if (comentario != NULL) *comentario = '\0';

        token_actual = strtok(linea_actual, " ,\n\r\t[]+*");
    }

    estandarizar_token(token_actual);
    strcpy(t.lexema, token_actual);
    t.tipo = clasificar_lexema(t.lexema);
    
    token_actual = strtok(NULL, " ,\n\r\t[]+*");
    return t;
}