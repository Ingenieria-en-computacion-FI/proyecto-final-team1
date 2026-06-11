#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/backend.h"
#include "../include/encoder.h"

void ejecutar_segunda_pasada(Token* token_list,
                              uint8_t* binario_final,
                              uint32_t* tamanio_total)
{
    Token* current      = token_list;
    uint32_t bytes_escritos = 0;

    printf("[PASADA 2] Iniciando generacion de codigo maquina...\n");

    while (current->tipo != TOKEN_EOF)
    {
        if (current->lexema[0] == '\0') { advance(&current); continue; }

        if (current->tipo == TOKEN_ETIQUETA) {
            advance(&current);
            continue;
        }

        if (strcmp(current->lexema, "DB") == 0 ||
            strcmp(current->lexema, "DW") == 0 ||
            strcmp(current->lexema, "DD") == 0)
        {
            Directive dir = parse_directive(&current);
            for (int i = 0; i < dir.arg_count; i++) {
                uint32_t valor = (uint32_t)atoi(dir.arguments[i]);
                if (strcmp(dir.name, "DB") == 0) {
                    binario_final[bytes_escritos++] = (uint8_t)(valor & 0xFF);
                } else if (strcmp(dir.name, "DW") == 0) {
                    binario_final[bytes_escritos++] = (uint8_t)(valor & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 8) & 0xFF);
                } else if (strcmp(dir.name, "DD") == 0) {
                    binario_final[bytes_escritos++] = (uint8_t)(valor & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 8) & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 16) & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 24) & 0xFF);
                }
            }
            continue;
        }

        if (strcmp(current->lexema, "SECTION") == 0 ||
            strcmp(current->lexema, "GLOBAL")  == 0 ||
            strcmp(current->lexema, "EXTERN")  == 0)
        {
            parse_directive(&current);
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
                    uint32_t dir_destino = (sym_idx != -1)
                        ? symbol_table[sym_idx].address : 0;

                    Instruction inst_salto;
                    memset(&inst_salto, 0, sizeof(inst_salto));
                    strncpy(inst_salto.mnemonic, mnemonic,
                            sizeof(inst_salto.mnemonic)-1);
                    inst_salto.operand_count  = 1;
                    inst_salto.op1.type       = OP_IMM;
                    inst_salto.op1.imm_value  = (int32_t)dir_destino;

                    uint32_t tam = codificar_instruccion_ia32(
                        &inst_salto, &binario_final[bytes_escritos]);
                    bytes_escritos += tam;

                    advance(&current);
                    advance(&current);
                    continue;
                }
            }

            Instruction inst = parse_instruction(&current);
            uint32_t tam = codificar_instruccion_ia32(
                &inst, &binario_final[bytes_escritos]);
            bytes_escritos += tam;
            continue;
        }

        advance(&current);
    }

    fixup_resolve_all(binario_final);

    *tamanio_total = bytes_escritos;
    printf("[PASADA 2] Completada. %u bytes generados.\n", bytes_escritos);
}
