#include <stdio.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/backend.h"

// Función para ejecutar la primera pasada sobre el flujo de tokens
void ejecutar_primera_pasada(Token* token_list) {
    Token* current = token_list;
    location_counter = 0;
    strcpy(current_section, ".text"); // Sección por defecto [cite: 210]

    while (current->type != TOK_EOF) {
        // Ignorar líneas vacías consecutivas
        if (current->type == TOK_NEWLINE) {
            advance(&current);
            continue;
        }

        // --- 1. DETECCIÓN DE ETIQUETAS ---
        // Si el token actual es un identificador y el que sigue es un carácter de dos puntos ':' 
        // (o si el lexer lo agrupa, asumimos aquí la convención estándar del token)
        if (current->type == TOK_IDENTIFIER && strcmp((current + 1)->value, ":") == 0) {
            // Es una etiqueta (ej. "inicio:"). La registramos en la tabla de símbolos[cite: 188].
            symbol_insert(current->value, location_counter, current_section, false, false);
            advance(&current); // Consumir el identificador
            advance(&current); // Consumir los ':'
            continue;
        }

        // --- 2. DETECCIÓN DE DIRECTIVAS (.data, .text, DB, DD) ---
        if (current->type == TOK_IDENTIFIER && 
           (strcmp(current->value, "SECTION") == 0 || strcmp(current->value, "DB") == 0 || 
            strcmp(current->value, "DW") == 0      || strcmp(current->value, "DD") == 0 ||
            strcmp(current->value, "GLOBAL") == 0   || strcmp(current->value, "EXTERN") == 0)) {
            
            Directive dir = parse_directive(&current); // Llamamos a la función de tu compañero
            
            if (strcmp(dir.name, "SECTION") == 0) {
                strncpy(current_section, dir.arguments[0], sizeof(current_section) - 1); // Cambiar sección [cite: 115]
            } 
            else if (strcmp(dir.name, "DB") == 0) {
                location_counter += (1 * dir.arg_count); // 1 byte por cada argumento [cite: 121]
            } 
            else if (strcmp(dir.name, "DW") == 0) {
                location_counter += (2 * dir.arg_count); // 2 bytes por cada argumento [cite: 123]
            } 
            else if (strcmp(dir.name, "DD") == 0) {
                location_counter += (4 * dir.arg_count); // 4 bytes por cada argumento [cite: 125]
            }
            else if (strcmp(dir.name, "GLOBAL") == 0) {
                // Registrar el símbolo como global si ya existe o prepararlo
                int idx = symbol_lookup(dir.arguments[0]);
                if (idx != -1) symbol_table[idx].is_global = true;
            }
            else if (strcmp(dir.name, "EXTERN") == 0) {
                // Registrar símbolo externo proveniente de otro módulo [cite: 119]
                symbol_insert(dir.arguments[0], 0, "extern", true, false);
            }

            if (current->type == TOK_NEWLINE) advance(&current);
            continue;
        }

        // --- 3. DETECCIÓN DE INSTRUCCIONES (MOV, ADD, JMP...) ---
        if (current->type == TOK_IDENTIFIER) {
            Token* pseudo_check = current;
            
            // Caso especial para saltos o llamadas: JMP etiqueta / CALL etiqueta
            // Como "etiqueta" no es un número ni un registro, parse_operand fallaría. 
            // Lo interceptamos aquí para tu Backend.
            if (strcmp(pseudo_check->value, "JMP") == 0 || pseudo_check->value[0] == 'J' || 
                strcmp(pseudo_check->value, "CALL") == 0) {
                
                char mnemonic[10];
                strcpy(mnemonic, pseudo_check->value);
                advance(&pseudo_check); // saltar mnemónico
                
                if (pseudo_check->type == TOK_IDENTIFIER && !is_register(pseudo_check->value)) {
                    // ¡Es una referencia a un símbolo/etiqueta!
                    char target_symbol[32];
                    strcpy(target_symbol, pseudo_check->value);
                    
                    int sym_idx = symbol_lookup(target_symbol);
                    if (sym_idx == -1) {
                        // REFERENCIA ADELANTADA: El símbolo no existe aún, agregamos un FIXUP 
                        // El parche se aplicará en el desplazamiento de la instrucción (location_counter + 1)
                        fixup_add(target_symbol, location_counter + 1, 4, current_section);
                    }
                    
                    uint32_t size = estimate_instruction_size(mnemonic, 1);
                    location_counter += size;
                    
                    current = pseudo_check; // Actualizar el puntero real de tokens
                    advance(&current);      // Consumir el identificador de la etiqueta
                    if (current->type == TOK_NEWLINE) advance(&current);
                    continue;
                }
            }

            // Flujo normal para instrucciones estándar (ej: MOV EAX, EBX) [cite: 141]
            Instruction inst = parse_instruction(&current); // Llamamos a la función del Integrante 2 
            uint32_t size = estimate_instruction_size(inst.mnemonic, inst.operand_count);
            location_counter += size;
            
            if (current->type == TOK_NEWLINE) advance(&current);
            continue;
        }

        // Avanzar por seguridad si hay un token huérfano
        advance(&current);
    }
}