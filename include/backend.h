#ifndef BACKEND_H
#define BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_SYMBOLS 256
#define MAX_FIXUPS 128

// Estructura para la Tabla de Símbolos (Etiquetas, variables, constantes)
typedef struct {
    char name[32];        // Nombre del símbolo (ej. "inicio", "mi_var")
    uint32_t address;     // Dirección (Location Counter) asociada
    char section[10];     // Sección a la que pertenece (".text", ".data", ".bss")
    bool is_extern;       // Declarado con EXTERN
    bool is_global;       // Declarado con GLOBAL
} Symbol;

// Estructura para manejar referencias adelantadas (Fixups)
typedef struct {
    char symbol_name[32]; // Símbolo indeterminado que estamos esperando
    uint32_t patch_pos;   // Posición en el buffer de salida/archivo donde se inyectará el offset
    uint32_t size;        // Tamaño del campo (generalmente 1, 2 o 4 bytes en IA-32)
    char section[10];     // Sección donde se requiere el parche
} Fixup;

// Variables globales del backend (compartidas)
extern Symbol symbol_table[MAX_SYMBOLS];
extern int symbol_count;

extern Fixup fixup_table[MAX_FIXUPS];
extern int fixup_count;

extern uint32_t location_counter;
extern char current_section[10];

// --- FUNCIONES DE LA TABLA DE SÍMBOLOS ---
int symbol_lookup(const char* name);
bool symbol_insert(const char* name, uint32_t address, const char* section, bool is_extern, bool is_global);

// --- FUNCIONES DE MANEJO DE FIXUPS ---
void fixup_add(const char* symbol_name, uint32_t patch_pos, uint32_t size, const char* section);
void fixup_resolve_all(uint8_t* code_buffer);

// --- LOGICA DE ESTIMACIÓN DE TAMAÑOS (Para calcular el Location Counter) ---
uint32_t estimate_instruction_size(const char* mnemonic, int op_count);

#endif // BACKEND_H