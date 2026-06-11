#include "../include/linker.h"
#include "../include/object_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MODULES 16
#define MAX_FINAL_BINARY 65536

/*
 * Tabla global de símbolos del linker.
 *
 * Aquí se fusionarán los símbolos exportados
 * por todos los módulos objeto.
 */
typedef struct
{
    char name[32];
    uint32_t address;
} GlobalSymbol;

static GlobalSymbol global_table[512];
static int global_count = 0;

/*
 * Busca un símbolo dentro de la tabla global
 * construida durante el enlace.
 */
static int buscar_global(const char* name)
{
    for(int i=0;i<global_count;i++)
    {
        if(strcmp(global_table[i].name,name)==0)
            return i;
    }

    return -1;
}

/*
 * Función principal del linker.
 *
 * Flujo:
 *
 * 1) Leer todos los objetos
 * 2) Construir tabla global
 * 3) Resolver símbolos externos
 * 4) Aplicar relocaciones
 * 5) Generar binario final
 *
 * NOTA:
 * Actualmente la implementación realiza
 * la fusión básica de módulos.
 *
 * Las fases 2, 3 y 4 están preparadas para
 * ampliarse cuando el Integrante 3 termine
 * la lógica completa de símbolos externos.
 */
int link_objects(
    const char* output_file,
    const char** object_files,
    int object_count
)
{
    FILE* out;

    uint8_t final_binary[MAX_FINAL_BINARY];

    uint32_t final_size = 0;

    for(int m=0;m<object_count;m++)
    {
        FILE* fp = fopen(object_files[m],"rb");

        if(!fp)
        {
            printf(
                "No se pudo abrir %s\n",
                object_files[m]
            );

            return 0;
        }

        ObjectHeader hdr;

        fread(&hdr,sizeof(hdr),1,fp);

        if(hdr.magic != OBJ_MAGIC)
        {
            printf(
                "Formato objeto invalido\n"
            );

            fclose(fp);
            return 0;
        }

        /*
         * Copiar el código máquina del módulo
         * al binario final.
         */
        fread(
            final_binary + final_size,
            1,
            hdr.code_size,
            fp
        );

        final_size += hdr.code_size;

        fclose(fp);
    }

    out = fopen(output_file,"wb");

    if(!out)
    {
        printf(
            "No se pudo crear %s\n",
            output_file
        );

        return 0;
    }

    fwrite(
        final_binary,
        1,
        final_size,
        out
    );

    fclose(out);

    printf(
        "[LINKER] Binario final generado: %s (%u bytes)\n",
        output_file,
        final_size
    );

    return 1;
}