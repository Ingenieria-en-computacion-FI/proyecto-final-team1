#include "../include/backend.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Definición de variables globales (Tablas del Integrante 3)
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

Fixup fixup_table[MAX_FIXUPS];
int fixup_count = 0;

uint32_t location_counter = 0;
char current_section[10] = ".text"; // Sección por defecto obligatoria

// Control de Location Counters por sección (.text, .data, .bss)
uint32_t section_lc[3] = {0, 0, 0}; 
int indice_seccion_actual = 0; // 0 = .text, 1 = .data, 2 = .bss

// Busca un símbolo en la tabla. Devuelve el índice o -1 si no existe.
int symbol_lookup(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Inserta un nuevo símbolo. Retorna false si hay redefinición (Manejo de Errores Semánticos).
bool symbol_insert(const char* name, uint32_t address, const char* section, bool is_extern, bool is_global) {
    int idx = symbol_lookup(name);
    if (idx != -1) {
        // Error: Redefinición de símbolo detectada en la primera pasada
        printf("Error Semantico: El simbolo '%s' ya fue definido.\n", name);
        return false; 
    }
    
    if (symbol_count >= MAX_SYMBOLS) {
        printf("Error: Tabla de simbolos llena.\n");
        return false;
    }

    strcpy(symbol_table[symbol_count].name, name);
    symbol_table[symbol_count].address = address;
    strcpy(symbol_table[symbol_count].section, section);
    symbol_table[symbol_count].is_extern = is_extern;
    symbol_table[symbol_count].is_global = is_global;
    
    symbol_count++;
    return true;
}

// Registra una referencia adelantada para parcharla posteriormente
void fixup_add(const char* symbol_name, uint32_t patch_pos, uint32_t size, const char* section) {
    if (fixup_count >= MAX_FIXUPS) {
        printf("Error: Tabla de fixups llena.\n");
        return;
    }
    
    strcpy(fixup_table[fixup_count].symbol_name, symbol_name);
    fixup_table[fixup_count].patch_pos = patch_pos;
    fixup_table[fixup_count].size = size;
    strcpy(fixup_table[fixup_count].section, section);
    
    fixup_count++;
}

// Resuelve todos los parches pendientes modificando el buffer de bytes del Encoder (IA-32)
void fixup_resolve_all(uint8_t* code_buffer) {
    for (int i = 0; i < fixup_count; i++) {
        int sym_idx = symbol_lookup(fixup_table[i].symbol_name);
        if (sym_idx == -1) {
            printf("Error de Enlazado/Ensamblado: Simbolo '%s' no resuelto.\n", fixup_table[i].symbol_name);
            continue;
        }

        uint32_t target_address = symbol_table[sym_idx].address;
        uint32_t patch_pos = fixup_table[i].patch_pos;
        uint32_t valor_a_inyectar = target_address;

        // CORRECCIÓN IA-32: Si el parche ocurre en la sección ejecutable (.text), 
        // calculamos el desplazamiento relativo en lugar de la dirección absoluta.
        if (strcmp(fixup_table[i].section, ".text") == 0) {
            valor_a_inyectar = target_address - (patch_pos + fixup_table[i].size);
        }

        // Inyectamos el valor en formato Little Endian dentro del buffer del código máquina
        if (fixup_table[i].size == 1) {
            code_buffer[patch_pos] = (uint8_t)(valor_a_inyectar & 0xFF);
        } else if (fixup_table[i].size == 4) {
            code_buffer[patch_pos]     = (uint8_t)(valor_a_inyectar & 0xFF);
            code_buffer[patch_pos + 1] = (uint8_t)((valor_a_inyectar >> 8) & 0xFF);
            code_buffer[patch_pos + 2] = (uint8_t)((valor_a_inyectar >> 16) & 0xFF);
            code_buffer[patch_pos + 3] = (uint8_t)((valor_a_inyectar >> 24) & 0xFF);
        }
        printf("Fixup aplicado con exito: Simbolo '%s' en pos %u con valor relativo %d\n", 
               fixup_table[i].symbol_name, patch_pos, (int32_t)valor_a_inyectar);
    }
}

// Administra y segmenta los contadores de posición individuales por sección (.text, .data, .bss)
void cambiar_de_seccion(const char* nueva_seccion) {
    // 1. Guardar el progreso del Location Counter de la sección activa
    section_lc[indice_seccion_actual] = location_counter;

    // 2. Mapear y cambiar al contexto de la nueva sección
    strncpy(current_section, nueva_seccion, sizeof(current_section) - 1);
    
    if (strcmp(nueva_seccion, ".text") == 0) {
        indice_seccion_actual = 0;
    } else if (strcmp(nueva_seccion, ".data") == 0) {
        indice_seccion_actual = 1;
    } else if (strcmp(nueva_seccion, ".bss") == 0) {
        indice_seccion_actual = 2;
    } else {
        printf("Error: Seccion '%s' no reconocida.\n", nueva_seccion);
        return;
    }

    // 3. Restaurar el Location Counter acumulado en el nuevo bloque destino
    location_counter = section_lc[indice_seccion_actual];
    printf("[BACKEND] Cambiando a seccion %s. LC restaurado en: %u\n", current_section, location_counter);
}

// Función auxiliar para estimar preliminarmente el peso de la instrucción en la Pasada 1
uint32_t estimate_instruction_size(const char* mnemonic, int op_count) {
    if (strcmp(mnemonic, "NOP") == 0 || strcmp(mnemonic, "RET") == 0) {
        return 1; // Opcodes de un solo byte
    }
    if (strcmp(mnemonic, "JMP") == 0 || mnemonic[0] == 'J') { 
        // Estimación estándar de IA-32 para saltos condicionales y absolutos directos largos
        return 5; // 1 byte de Opcode + 4 bytes de desplazamiento relativo
    }
    if (strcmp(mnemonic, "MOV") == 0) {
        if (op_count == 2) return 5; // Opcode + ModRM + Inmediato/Desplazamiento base
    }
    return 3; // Estimación genérica de resguardo para desarrollo estructural
}