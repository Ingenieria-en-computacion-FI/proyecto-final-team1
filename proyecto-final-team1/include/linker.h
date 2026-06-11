#ifndef LINKER_H
#define LINKER_H

/*
 * Linker del proyecto IA-32.
 *
 * Responsabilidades:
 *  - leer múltiples archivos objeto
 *  - resolver símbolos globales
 *  - resolver símbolos externos
 *  - aplicar relocaciones
 *  - generar binario final
 */

int link_objects(
    const char* output_file,
    const char** object_files,
    int object_count
);

#endif