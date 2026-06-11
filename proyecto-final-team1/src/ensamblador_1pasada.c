#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/backend.h"

void ejecutar_primera_pasada(Token* token_list)
{
    Token* current = token_list;
    location_counter = 0;
    strcpy(current_section, ".text");

    while (current->tipo != TOKEN_EOF)
    {
        if (current->lexema[0] == '\0') { advance(&current); continue; }

        if (current->tipo == TOKEN_ETIQUETA)
        {
            char nombre[64];
            strncpy(nombre, current->lexema, sizeof(nombre) - 1);
            int len = strlen(nombre);
            if (len > 0 && nombre[len-1] == ':')
                nombre[len-1] = '\0';

            // Escaneo predictivo para hacer que GLOBAL funcione arriba o abajo de la etiqueta
            bool deba_ser_global = false;
            Token* scan = token_list;
            while (scan->tipo != TOKEN_EOF) {
                if (strcmp(scan->lexema, "GLOBAL") == 0) {
                    Token* arg = scan + 1;
                    if (arg->tipo != TOKEN_EOF && strcmp(arg->lexema, nombre) == 0) {
                        deba_ser_global = true;
                        break;
                    }
                }
                scan++;
            }

            symbol_insert(nombre, location_counter, current_section, false, deba_ser_global);
            advance(&current);
            continue;
        }

        if (strcmp(current->lexema, "SECTION") == 0 ||
            strcmp(current->lexema, "DB")      == 0 ||
            strcmp(current->lexema, "DW")      == 0 ||
            strcmp(current->lexema, "DD")      == 0 ||
            strcmp(current->lexema, "GLOBAL")  == 0 ||
            strcmp(current->lexema, "EXTERN")  == 0)
        {
            Directive dir = parse_directive(&current);

            if (strcmp(dir.name, "SECTION") == 0) {
                cambiar_de_seccion(dir.arguments[0]);
            }
            else if (strcmp(dir.name, "DB") == 0) {
                location_counter += 1 * dir.arg_count;
            }
            else if (strcmp(dir.name, "DW") == 0) {
                location_counter += 2 * dir.arg_count;
            }
            else if (strcmp(dir.name, "DD") == 0) {
                location_counter += 4 * dir.arg_count;
            }
            else if (strcmp(dir.name, "GLOBAL") == 0) {
                int idx = symbol_lookup(dir.arguments[0]);
                if (idx != -1) symbol_table[idx].is_global = true;
            }
            else if (strcmp(dir.name, "EXTERN") == 0) {
                symbol_insert(dir.arguments[0], 0, "extern", true, false);
            }
            continue;
        }

        if (current->tipo == TOKEN_IDENTIFICADOR ||
            (current->tipo >= INST_MOV && current->tipo <= INST_INT))
        {
            if (strcmp(current->lexema, "JMP")  == 0 ||
                strcmp(current->lexema, "CALL") == 0 ||
                current->lexema[0] == 'J')
            {
                char mnemonic[10];
                strncpy(mnemonic, current->lexema, sizeof(mnemonic)-1);
                Token* peek = current + 1;

                if (peek->tipo == TOKEN_IDENTIFICADOR &&
                    !is_register(peek->lexema))
                {
                    int sym_idx = symbol_lookup(peek->lexema);
                    if (sym_idx == -1)
                        fixup_add(peek->lexema, location_counter + 1,
                                  4, current_section);

                    location_counter += estimate_instruction_size(mnemonic, 1);
                    advance(&current);
                    advance(&current);
                    continue;
                }
            }

            Instruction inst = parse_instruction(&current);
            location_counter += estimate_instruction_size(
                inst.mnemonic, inst.operand_count);
            continue;
        }

        advance(&current);
    }

    printf("[PASADA 1] Completada. %d simbolos registrados. LC final: %u\n",
           symbol_count, location_counter);
}
