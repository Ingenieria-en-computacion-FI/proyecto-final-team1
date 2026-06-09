#include "../include/object_file.h"
#include "../include/backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Esta función es llamada al terminar la
 * Segunda Pasada del ensamblador.
 *
 * Entradas:
 *  - código máquina generado por el Integrante 4
 *  - tabla de símbolos del Integrante 3
 *  - tabla de fixups del Integrante 3
 *
 * Salida:
 *  - archivo .obj listo para el linker
 */
int write_object_file(
    const char* filename,
    uint8_t* code,
    uint32_t code_size
)
{
    FILE* fp = fopen(filename,"wb");

    if(!fp)
    {
        printf("Error creando %s\n",filename);
        return 0;
    }

    ObjectHeader hdr;

    hdr.magic = OBJ_MAGIC;
    hdr.code_size = code_size;
    hdr.symbol_count = symbol_count;
    hdr.relocation_count = fixup_count;

    /* Escribir encabezado */
    fwrite(&hdr,sizeof(hdr),1,fp);

    /* Escribir código máquina */
    fwrite(code,1,code_size,fp);

    /* Exportar tabla de símbolos */
    for(int i=0;i<symbol_count;i++)
    {
        ObjectSymbol sym;

        memset(&sym,0,sizeof(sym));

        strcpy(sym.name,symbol_table[i].name);

        sym.address =
            symbol_table[i].address;

        sym.is_global =
            symbol_table[i].is_global;

        sym.is_extern =
            symbol_table[i].is_extern;

        fwrite(&sym,sizeof(sym),1,fp);
    }

    /* Exportar tabla de relocaciones */
    for(int i=0;i<fixup_count;i++)
    {
        RelocationEntry rel;

        memset(&rel,0,sizeof(rel));

        strcpy(
            rel.symbol_name,
            fixup_table[i].symbol_name
        );

        rel.patch_pos =
            fixup_table[i].patch_pos;

        rel.size =
            fixup_table[i].size;

        fwrite(&rel,sizeof(rel),1,fp);
    }

    fclose(fp);

    printf(
        "[OBJ] Archivo objeto generado: %s\n",
        filename
    );

    return 1;
}

/*
 * Utilidad de depuración.
 *
 * Permite inspeccionar rápidamente un archivo
 * objeto sin necesidad de enlazarlo.
 */
void dump_object_file(const char* filename)
{
    FILE* fp = fopen(filename,"rb");

    if(!fp)
    {
        printf("No se pudo abrir %s\n",filename);
        return;
    }

    ObjectHeader hdr;

    fread(&hdr,sizeof(hdr),1,fp);

    printf("\n===== OBJECT DUMP =====\n");

    printf("Magic       : %X\n",hdr.magic);
    printf("Code Size   : %u\n",hdr.code_size);
    printf("Symbols     : %u\n",hdr.symbol_count);
    printf("Relocations : %u\n",hdr.relocation_count);

    fclose(fp);
}