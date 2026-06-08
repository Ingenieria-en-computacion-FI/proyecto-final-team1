#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Función auxiliar para avanzar al siguiente token
void advance(Token** current) {
    (*current)++;
}

// Verifica si un string es un registro válido de IA-32
bool is_register(const char* str) {
    const char* regs[] = {"EAX", "EBX", "ECX", "EDX", "ESI", "EDI", "EBP", "ESP"};
    for (int i = 0; i < 8; i++) {
        if (strcmp(str, regs[i]) == 0) return true;
    }
    return false;
}

// Parsea un operando individual
Operand parse_operand(Token** current) {
    Operand op;
    memset(&op, 0, sizeof(Operand)); // Inicializar en ceros
    op.type = OP_NONE;
    op.scale = 1; // Escala por defecto

    Token* tok = *current;

    // 1. Modo Inmediato
    if (tok->type == TOK_NUMBER) {
        op.type = OP_IMM;
        op.imm_value = atoi(tok->value);
        advance(current);
        return op;
    }

    // 2. Registro a registro
    if (tok->type == TOK_IDENTIFIER && is_register(tok->value)) {
        op.type = OP_REG;
        strcpy(op.reg_name, tok->value);
        advance(current);
        return op;
    }

    // 3. Modos de memoria (Requiere corchetes)
    if (tok->type == TOK_LBRACKET) {
        return parse_memory_operand(current);
    }

    // Manejo de errores básicos (Integrante 2)
    printf("Error de sintaxis: Operando no reconocido '%s'\n", tok->value);
    exit(1);
}

// Parsea el interior de los corchetes: [Base + Indice*Escala + Desplazamiento]
Operand parse_memory_operand(Token** current) {
    Operand mem_op;
    memset(&mem_op, 0, sizeof(Operand));
    mem_op.type = OP_MEM;
    mem_op.scale = 1;
    
    advance(current); // Consumir '['

    while ((*current)->type != TOK_RBRACKET && (*current)->type != TOK_EOF) {
        Token* tok = *current;

        if (tok->type == TOK_IDENTIFIER && is_register(tok->value)) {
            // Lógica SIB simplificada
            if (!mem_op.has_base) {
                strcpy(mem_op.base_reg, tok->value);
                mem_op.has_base = true;
                advance(current);
            } else if (!mem_op.has_index) {
                strcpy(mem_op.index_reg, tok->value);
                mem_op.has_index = true;
                advance(current);
                
                // Verificar si hay escala (Ej: * 4)
                if ((*current)->type == TOK_STAR) {
                    advance(current); // Consumir '*'
                    if ((*current)->type == TOK_NUMBER) {
                        mem_op.scale = atoi((*current)->value);
                        advance(current);
                    }
                }
            }
        } 
        else if (tok->type == TOK_PLUS) {
            advance(current); // Ignorar el '+' y continuar evaluando
        }
        else if (tok->type == TOK_NUMBER) {
            // Es un desplazamiento estático (Ej: + 8)
            mem_op.displacement += atoi(tok->value);
            advance(current);
        } else {
            advance(current); // Avanzar en caso de token no esperado para no colgarse
        }
    }

    if ((*current)->type == TOK_RBRACKET) {
        advance(current); // Consumir ']'
    } else {
        printf("Error: Se esperaba ']'\n");
        exit(1);
    }

    return mem_op;
}

// Parsea una instrucción completa (Mnemónico + Operandos)
Instruction parse_instruction(Token** current) {
    Instruction inst;
    memset(&inst, 0, sizeof(Instruction)); // Limpiar estructura
    
    // 1. Guardar el mnemónico (ej. "MOV", "ADD")
    strncpy(inst.mnemonic, (*current)->value, sizeof(inst.mnemonic) - 1);
    advance(current);
    
    // 2. Revisar si hay operandos antes de que acabe la línea
    if ((*current)->type != TOK_NEWLINE && (*current)->type != TOK_EOF) {
        
        // Parsear el primer operando (destino)
        inst.op1 = parse_operand(current);
        inst.operand_count = 1;
        
        // 3. Validación sintáctica: Si hay más texto, DEBE haber una coma
        if ((*current)->type == TOK_COMMA) {
            advance(current); // Consumir la coma
            
            // Parsear el segundo operando (origen)
            inst.op2 = parse_operand(current);
            inst.operand_count = 2;
        } else if ((*current)->type != TOK_NEWLINE && (*current)->type != TOK_EOF) {
            printf("Error de sintaxis: Se esperaba una coma ',' después del operando.\n");
            exit(1);
        }
    }
    
    return inst;
}

// Parsea directivas de ensamblador
Directive parse_directive(Token** current) {
    Directive dir;
    memset(&dir, 0, sizeof(Directive));
    
    // Guardar el nombre de la directiva (ej. "SECTION", "DB")
    strncpy(dir.name, (*current)->value, sizeof(dir.name) - 1);
    advance(current);
    
    // Leer argumentos hasta encontrar un salto de línea o EOF
    while ((*current)->type != TOK_NEWLINE && (*current)->type != TOK_EOF) {
        // Ignorar comas si la directiva tiene varios parámetros (Ej: DB 10, 20)
        if ((*current)->type == TOK_COMMA) {
            advance(current);
            continue;
        }
        
        if (dir.arg_count < 4) {
            strncpy(dir.arguments[dir.arg_count], (*current)->value, 31);
            dir.arg_count++;
        }
        advance(current);
    }
    
    return dir;
}
