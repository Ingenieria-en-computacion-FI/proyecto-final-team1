#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#include <stdint.h>

/*
 * FORMATO OBJETO PROPIO DEL PROYECTO
 *
 * Estructura física del archivo:
 *
 * [HEADER]
 * [CODIGO MAQUINA]
 * [TABLA DE SIMBOLOS]
 * [TABLA DE RELOCACIONES]
 *
 * Este formato será utilizado por el Linker para
 * combinar múltiples módulos ensamblados.
 */

#define OBJ_MAGIC 0x31424F4A

/* Encabezado principal del archivo objeto */
typedef struct {
    uint32_t magic;
    uint32_t code_size;
    uint32_t symbol_count;
    uint32_t relocation_count;
} ObjectHeader;

/*
 * Copia serializable de Symbol.
 *
 * Se mantiene independiente del backend para que
 * el archivo pueda ser leído posteriormente.
 */
typedef struct {
    char name[32];
    uint32_t address;
    uint8_t is_global;
    uint8_t is_extern;
} ObjectSymbol;

/*
 * Entrada de relocación.
 *
 * Indica al linker:
 *  - qué símbolo buscar
 *  - dónde aplicar el parche
 *  - cuántos bytes modificar
 */
typedef struct {
    char symbol_name[32];
    uint32_t patch_pos;
    uint32_t size;
} RelocationEntry;

/* Escritura de archivo objeto */
int write_object_file(
    const char* filename,
    uint8_t* code,
    uint32_t code_size
);

/* Lectura de archivo objeto */
int read_object_file(
    const char* filename,
    uint8_t** code,
    uint32_t* code_size
);

/* Herramienta de depuración */
void dump_object_file(const char* filename);

#endif