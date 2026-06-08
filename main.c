#include <stdio.h>
#include "tokens.h"

int main() {
    printf("--- Iniciando Modulo Lexer (Integrante 1) ---\n\n");
    
    // Busca el archivo de prueba en la raíz del proyecto
    iniciar_lexer("test.asm");

    Token t = obtener_siguiente_token();

    while (t.tipo != TOKEN_EOF) {
        printf("Lexema detectado: %-15s | ID Enum: %d\n", t.lexema, t.tipo);
        t = obtener_siguiente_token();
    }

    cerrar_lexer();
    printf("\n--- Lectura de archivo ASM finalizada ---\n");
    return 0;
}