#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "encoder.h"

static int reg_code(const char* reg);
static void write_imm32(uint8_t* buffer, int32_t value);
static void write_disp32(uint8_t* buffer, int32_t value);
static uint8_t make_modrm(uint8_t mod, uint8_t reg, uint8_t rm);
static uint8_t make_sib(uint8_t scale, uint8_t index, uint8_t base);

uint32_t codificar_instruccion_ia32(
    Instruction* inst,
    uint8_t* output_buffer
)
{
    /* NOP — 90 */
    if (strcmp(inst->mnemonic, "NOP") == 0)
    {
        output_buffer[0] = 0x90;
        return 1;
    }

    /* RET — C3 */
    if (strcmp(inst->mnemonic, "RET") == 0)
    {
        output_buffer[0] = 0xC3;
        return 1;
    }

    /* PUSH reg — 50+r */
    if (strcmp(inst->mnemonic, "PUSH") == 0)
    {
        if (inst->operand_count == 1 && inst->op1.type == OP_REG)
        {
            output_buffer[0] = 0x50 + reg_code(inst->op1.reg_name);
            return 1;
        }
    }

    /* POP reg — 58+r */
    if (strcmp(inst->mnemonic, "POP") == 0)
    {
        if (inst->operand_count == 1 && inst->op1.type == OP_REG)
        {
            output_buffer[0] = 0x58 + reg_code(inst->op1.reg_name);
            return 1;
        }
    }

    /* MOV reg, imm32 — B8+r id */
    if (strcmp(inst->mnemonic, "MOV") == 0 &&
        inst->operand_count == 2 &&
        inst->op1.type == OP_REG &&
        inst->op2.type == OP_IMM)
    {
        output_buffer[0] = 0xB8 + reg_code(inst->op1.reg_name);
        write_imm32(&output_buffer[1], inst->op2.imm_value);
        return 5;
    }

    /* MOV reg, reg — 89 /r */
    if (strcmp(inst->mnemonic, "MOV") == 0 &&
        inst->operand_count == 2 &&
        inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);
        output_buffer[0] = 0x89;
        output_buffer[1] = 0xC0 | (src << 3) | dst;
        return 2;
    }

    /* MOV reg, [mem+disp] — 8B /r disp32 */
    if (strcmp(inst->mnemonic, "MOV") == 0 &&
        inst->op1.type == OP_REG &&
        inst->op2.type == OP_MEM)
    {
        int dst = reg_code(inst->op1.reg_name);
        int pos = 0;
        output_buffer[pos++] = 0x8B;
        if (inst->op2.has_base && !inst->op2.has_index)
        {
            int base = reg_code(inst->op2.base_reg);
            output_buffer[pos++] = make_modrm(2, dst, base);
            write_disp32(&output_buffer[pos], inst->op2.displacement);
            pos += 4;
            return pos;
        }
    }

    /* MOV [mem+disp], reg — 89 /r disp32 */
    if (strcmp(inst->mnemonic, "MOV") == 0 &&
        inst->op1.type == OP_MEM &&
        inst->op2.type == OP_REG)
    {
        int src = reg_code(inst->op2.reg_name);
        int pos = 0;
        output_buffer[pos++] = 0x89;
        if (inst->op1.has_base && !inst->op1.has_index)
        {
            int base = reg_code(inst->op1.base_reg);
            output_buffer[pos++] = make_modrm(2, src, base);
            write_disp32(&output_buffer[pos], inst->op1.displacement);
            pos += 4;
            return pos;
        }
    }

    /* ADD reg, reg — 01 /r */
    if (strcmp(inst->mnemonic, "ADD") == 0 &&
        inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);
        output_buffer[0] = 0x01;
        output_buffer[1] = make_modrm(3, src, dst);
        return 2;
    }

    /* SUB reg, reg — 29 /r */
    if (strcmp(inst->mnemonic, "SUB") == 0 &&
        inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);
        output_buffer[0] = 0x29;
        output_buffer[1] = make_modrm(3, src, dst);
        return 2;
    }

    /* CMP reg, reg — 39 /r */
    if (strcmp(inst->mnemonic, "CMP") == 0 &&
        inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);
        output_buffer[0] = 0x39;
        output_buffer[1] = make_modrm(3, src, dst);
        return 2;
    }

    /* JMP rel32 — E9 cd */
    if (strcmp(inst->mnemonic, "JMP") == 0)
    {
        output_buffer[0] = 0xE9;
        write_imm32(&output_buffer[1], inst->op1.imm_value);
        return 5;
    }

    /* CALL rel32 — E8 cd */
    if (strcmp(inst->mnemonic, "CALL") == 0)
    {
        output_buffer[0] = 0xE8;
        write_imm32(&output_buffer[1], inst->op1.imm_value);
        return 5;
    }

    /* JE rel32 — 0F 84 cd */
    if (strcmp(inst->mnemonic, "JE") == 0)
    {
        output_buffer[0] = 0x0F;
        output_buffer[1] = 0x84;
        write_imm32(&output_buffer[2], inst->op1.imm_value);
        return 6;
    }

    /* JNE rel32 — 0F 85 cd */
    if (strcmp(inst->mnemonic, "JNE") == 0)
    {
        output_buffer[0] = 0x0F;
        output_buffer[1] = 0x85;
        write_imm32(&output_buffer[2], inst->op1.imm_value);
        return 6;
    }

    /* INT imm8 — CD ib  (ej: INT 0x80) */
    if (strcmp(inst->mnemonic, "INT") == 0 &&
        inst->operand_count == 1 &&
        inst->op1.type == OP_IMM)
    {
        output_buffer[0] = 0xCD;
        output_buffer[1] = (uint8_t)(inst->op1.imm_value & 0xFF);
        return 2;
    }

    /* INC reg — 40+r */
    if (strcmp(inst->mnemonic, "INC") == 0 &&
        inst->operand_count == 1 &&
        inst->op1.type == OP_REG)
    {
        output_buffer[0] = 0x40 + reg_code(inst->op1.reg_name);
        return 1;
    }

    /* DEC reg — 48+r */
    if (strcmp(inst->mnemonic, "DEC") == 0 &&
        inst->operand_count == 1 &&
        inst->op1.type == OP_REG)
    {
        output_buffer[0] = 0x48 + reg_code(inst->op1.reg_name);
        return 1;
    }

    printf("[ENCODER] Instruccion no soportada: %s\n", inst->mnemonic);
    return 0;
}

/* ---- Funciones auxiliares ---- */

static void write_imm32(uint8_t* buffer, int32_t value)
{
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8)  & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
}

static void write_disp32(uint8_t* buffer, int32_t value)
{
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8)  & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
}

static uint8_t make_modrm(uint8_t mod, uint8_t reg, uint8_t rm)
{
    return (mod << 6) | (reg << 3) | rm;
}

static uint8_t make_sib(uint8_t scale, uint8_t index, uint8_t base)
{
    return (scale << 6) | (index << 3) | base;
}

static int reg_code(const char* reg)
{
    if (strcmp(reg, "EAX") == 0) return 0;
    if (strcmp(reg, "ECX") == 0) return 1;
    if (strcmp(reg, "EDX") == 0) return 2;
    if (strcmp(reg, "EBX") == 0) return 3;
    if (strcmp(reg, "ESP") == 0) return 4;
    if (strcmp(reg, "EBP") == 0) return 5;
    if (strcmp(reg, "ESI") == 0) return 6;
    if (strcmp(reg, "EDI") == 0) return 7;
    return 0;
}
