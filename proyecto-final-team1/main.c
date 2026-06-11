#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/tokens.h"
#include "include/parser.h"
#include "include/backend.h"
#include "include/encoder.h"
#include "include/object_file.h"
#include "include/ensamblador.h"

#define MAX_TOKENS   4096
#define MAX_BINARIO  65536

int main(int argc, char* argv[])
{
    const char* ruta_asm = (argc > 1) ? argv[1] : "examples/test.asm";

    printf("=== Ensamblador IA-32 — proyecto final ===\n\n");

    /* -------------------------------------------------------
     * PASO 1: Análisis léxico
     * ------------------------------------------------------- */
    printf("[LEXER] Abriendo: %s\n", ruta_asm);
    iniciar_lexer(ruta_asm);

    Token token_list[MAX_TOKENS];
    int   n = 0;

    Token t = obtener_siguiente_token();
    while (t.tipo != TOKEN_EOF && n < MAX_TOKENS - 1)
    {
        token_list[n++] = t;
        t = obtener_siguiente_token();
    }
    /* token centinela EOF */
    token_list[n].tipo    = TOKEN_EOF;
    token_list[n].lexema[0] = '\0';

    cerrar_lexer();
    printf("[LEXER] %d tokens extraidos.\n\n", n);

    /* -------------------------------------------------------
     * PASO 2: Primera pasada (tabla de símbolos + fixups)
     * ------------------------------------------------------- */
    ejecutar_primera_pasada(token_list);
    printf("\n");

    /* -------------------------------------------------------
     * PASO 3: Segunda pasada (generación de código máquina)
     * ------------------------------------------------------- */
    uint8_t  binario[MAX_BINARIO];
    uint32_t tamanio = 0;
    memset(binario, 0, sizeof(binario));

    ejecutar_segunda_pasada(token_list, binario, &tamanio);
    printf("\n");

    /* -------------------------------------------------------
     * PASO 4: Escribir archivo objeto
     * ------------------------------------------------------- */
    if (tamanio > 0)
    {
        const char* ruta_obj = "salida.obj";
        if (write_object_file(ruta_obj, binario, tamanio))
            printf("\n[MAIN] Archivo objeto generado: %s\n", ruta_obj);

        /* Volcar tabla de símbolos final */
        printf("\n--- Tabla de simbolos ---\n");
        for (int i = 0; i < symbol_count; i++)
        {
            printf("  %-20s  addr=0x%04X  sec=%-8s  global=%d  extern=%d\n",
                   symbol_table[i].name,
                   symbol_table[i].address,
                   symbol_table[i].section,
                   symbol_table[i].is_global,
                   symbol_table[i].is_extern);
        }

        /* Volcar primeros bytes del binario */
        printf("\n--- Codigo maquina (%u bytes) ---\n", tamanio);
        for (uint32_t i = 0; i < tamanio; i++)
        {
            printf("%02X ", binario[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");
    }
    else
    {
        printf("[MAIN] No se genero codigo (0 bytes).\n");
    }

    printf("\n=== Ensamblado finalizado ===\n");
    return 0;
}
