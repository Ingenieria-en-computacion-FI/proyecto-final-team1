#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

void advance(Token** current)
{
    (*current)++;
}

bool is_register(const char* str)
{
    const char* regs[] = {
        "EAX","EBX","ECX","EDX",
        "ESI","EDI","EBP","ESP"
    };
    for (int i = 0; i < 8; i++)
        if (strcmp(str, regs[i]) == 0) return true;
    return false;
}

Operand parse_memory_operand(Token** current)
{
    Operand mem_op;
    memset(&mem_op, 0, sizeof(Operand));
    mem_op.type  = OP_MEM;
    mem_op.scale = 1;

    advance(current); /* consumir '[' */

    while ((*current)->tipo != TOKEN_EOF)
    {
        if (strcmp((*current)->lexema, "]") == 0) {
            advance(current);
            break;
        }

        Token* tok = *current;

        if (tok->tipo >= REG_EAX && tok->tipo <= REG_ESP)
        {
            if (!mem_op.has_base) {
                strcpy(mem_op.base_reg, tok->lexema);
                mem_op.has_base = true;
                advance(current);
            } else if (!mem_op.has_index) {
                strcpy(mem_op.index_reg, tok->lexema);
                mem_op.has_index = true;
                advance(current);
                if (strcmp((*current)->lexema, "*") == 0) {
                    advance(current);
                    if ((*current)->tipo == TOKEN_INMEDIATO) {
                        mem_op.scale = atoi((*current)->lexema);
                        advance(current);
                    }
                }
            }
        }
        else if (tok->tipo == TOKEN_INMEDIATO)
        {
            mem_op.displacement += atoi(tok->lexema);
            advance(current);
        }
        else if (tok->tipo == TOKEN_IDENTIFICADOR)
        {
            strcpy(mem_op.base_reg, tok->lexema);
            mem_op.has_base = true;
            advance(current);
        }
        else
        {
            advance(current);
        }
    }

    return mem_op;
}

Operand parse_operand(Token** current)
{
    Operand op;
    memset(&op, 0, sizeof(Operand));
    op.type  = OP_NONE;
    op.scale = 1;

    Token* tok = *current;

    if (tok->tipo == TOKEN_INMEDIATO) {
        op.type      = OP_IMM;
        op.imm_value = atoi(tok->lexema);
        advance(current);
        return op;
    }

    if (tok->tipo >= REG_EAX && tok->tipo <= REG_ESP) {
        op.type = OP_REG;
        strcpy(op.reg_name, tok->lexema);
        advance(current);
        return op;
    }

    if (tok->tipo == TOKEN_IDENTIFICADOR && is_register(tok->lexema)) {
        op.type = OP_REG;
        strcpy(op.reg_name, tok->lexema);
        advance(current);
        return op;
    }

    if (strcmp(tok->lexema, "[") == 0) {
        return parse_memory_operand(current);
    }

    if (tok->tipo == TOKEN_IDENTIFICADOR) {
        op.type      = OP_IMM;
        op.imm_value = 0;
        advance(current);
        return op;
    }

    printf("Error de sintaxis: operando no reconocido '%s'\n", tok->lexema);
    advance(current);
    return op;
}

Instruction parse_instruction(Token** current)
{
    Instruction inst;
    memset(&inst, 0, sizeof(Instruction));

    strncpy(inst.mnemonic, (*current)->lexema, sizeof(inst.mnemonic) - 1);
    advance(current);

    if ((*current)->tipo == TOKEN_EOF ||
        (*current)->tipo == TOKEN_ETIQUETA)
        return inst;

    inst.op1 = parse_operand(current);
    inst.operand_count = 1;

    if ((*current)->tipo != TOKEN_EOF && (*current)->tipo != TOKEN_ETIQUETA) {
        inst.op2 = parse_operand(current);
        inst.operand_count = 2;
    }

    return inst;
}

Directive parse_directive(Token** current)
{
    Directive dir;
    memset(&dir, 0, sizeof(Directive));

    strncpy(dir.name, (*current)->lexema, sizeof(dir.name) - 1);
    advance(current);

    while ((*current)->tipo != TOKEN_EOF)
    {
        if (strcmp((*current)->lexema, ",") == 0) {
            advance(current);
            continue;
        }
        if ((*current)->tipo == TOKEN_ETIQUETA)
            break;

        if (strcmp((*current)->lexema, "SECTION") == 0 ||
            strcmp((*current)->lexema, "GLOBAL")  == 0 ||
            strcmp((*current)->lexema, "EXTERN")  == 0 ||
            strcmp((*current)->lexema, "MOV")     == 0 ||
            strcmp((*current)->lexema, "NOP")     == 0)
        {
            break;
        }

        if (dir.arg_count < 4) {
            strncpy(dir.arguments[dir.arg_count],
                    (*current)->lexema, 31);
            dir.arg_count++;
        }
        advance(current);
    }

    return dir;
}
