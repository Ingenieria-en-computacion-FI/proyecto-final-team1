#include "../include/parser.h"
#include "../include/backend.h"
#include <stdio.h>
#include <string.h>

// Simulamos la llamada al Encoder del Integrante 4
// En tu proyecto real, él te proveerá esta función o una similar
extern uint32_t codificar_instruccion_ia32(Instruction* inst, uint8_t* output_buffer);

void ejecutar_segunda_pasada(Token* token_list, uint8_t* binario_final, uint32_t* tamaño_total_binario) {
    Token* current = token_list;
    uint32_t pc_segunda_pasada = 0;
    uint32_t bytes_escritos = 0;

    printf("[BACKEND] Iniciando Segunda Pasada: Generacion de codigo maquina...\n");

    while (current->type != TOK_EOF) {
        if (current->type == TOK_NEWLINE) {
            advance(&current);
            continue;
        }

        // 1. Ignorar declaraciones de etiquetas (ya las procesamos en la pasada 1)
        if (current->type == TOK_IDENTIFIER && strcmp((current + 1)->value, ":") == 0) {
            advance(&current); // Consumir nombre
            advance(&current); // Consumir ':'
            continue;
        }

        // 2. Procesar Directivas de Datos (Inyectar valores reales al binario)
        if (current->type == TOK_IDENTIFIER && 
           (strcmp(current->value, "DB") == 0 || strcmp(current->value, "DW") == 0 || strcmp(current->value, "DD") == 0)) {
            
            Directive dir = parse_directive(&current);
            for (int i = 0; i < dir.arg_count; i++) {
                uint32_t valor = atoi(dir.arguments[i]);
                if (strcmp(dir.name, "DB") == 0) {
                    binario_final[bytes_escritos++] = (uint8_t)(valor & 0xFF);
                } else if (strcmp(dir.name, "DD") == 0) {
                    // Little Endian
                    binario_final[bytes_escritos++] = (uint8_t)(valor & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 8) & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 16) & 0xFF);
                    binario_final[bytes_escritos++] = (uint8_t)((valor >> 24) & 0xFF);
                }
            }
            if (current->type == TOK_NEWLINE) advance(&current);
            continue;
        }

        // 3. Procesar Instrucciones Reales
        if (current->type == TOK_IDENTIFIER) {
            // Si es un salto (JMP/CALL) con etiqueta, resolvemos su dirección usando tu tabla
            if (strcmp(current->value, "JMP") == 0 || current->value[0] == 'J' || strcmp(current->value, "CALL") == 0) {
                char mnemonic[10];
                strcpy(mnemonic, current->value);
                advance(&current);

                if (current->type == TOK_IDENTIFIER && !is_register(current->value)) {
                    int sym_idx = symbol_lookup(current->value);
                    uint32_t dir_destino = (sym_idx != -1) ? symbol_table[sym_idx].address : 0;

                    // Aquí creamos una estructura Instruction limpia para pasarle al Integrante 4
                    Instruction inst_salto;
                    strcpy(inst_salto.mnemonic, mnemonic);
                    inst_salto.operand_count = 1;
                    inst_salto.op1.type = OP_IMM;
                    inst_salto.op1.imm_value = (int32_t)dir_destino; // ¡Dirección resuelta!

                    // El Integrante 4 escribe los bytes en el buffer y nos dice cuánto midió
                    uint32_t tam_bytes = codificar_instruccion_ia32(&inst_salto, &binario_final[bytes_escritos]);
                    bytes_escritos += tam_bytes;

                    advance(&current);
                    if (current->type == TOK_NEWLINE) advance(&current);
                    continue;
                }
            }

            // Instrucciones estándar (MOV, ADD, etc.)
            Instruction inst = parse_instruction(&current);
            uint32_t tam_bytes = codificar_instruccion_ia32(&inst, &binario_final[bytes_escritos]);
            bytes_escritos += tam_bytes;

            if (current->type == TOK_NEWLINE) advance(&current);
            continue;
        }

        advance(&current);
    }

    *tamaño_total_binario = bytes_escritos;
    printf("[BACKEND] Segunda Pasada completada de forma limpia. Tamano binario: %u bytes.\n", bytes_escritos);
}