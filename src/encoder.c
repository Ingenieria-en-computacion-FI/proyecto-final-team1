#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "encoder.h"

static int reg_code(const char* reg);
static void write_imm32(uint8_t* buffer, int32_t value);

uint32_t codificar_instruccion_ia32(
    Instruction* inst,
    uint8_t* output_buffer
)
{
    uint32_t bytes = 0;

    /*
        NOP
        90
    */
    if (strcmp(inst->mnemonic, "NOP") == 0)
    {
        output_buffer[0] = 0x90;
        return 1;
    }

    /*
        RET
        C3
    */
    if (strcmp(inst->mnemonic, "RET") == 0)
    {
        output_buffer[0] = 0xC3;
        return 1;
    }

    /*
        PUSH reg
        50 + reg
    */
    if (strcmp(inst->mnemonic, "PUSH") == 0)
    {
        if (inst->operand_count == 1 &&
            inst->op1.type == OP_REG)
        {
            int reg = reg_code(inst->op1.reg_name);

            output_buffer[0] = 0x50 + reg;

            return 1;
        }
    }

    /*
        POP reg
        58 + reg
    */
    if (strcmp(inst->mnemonic, "POP") == 0)
    {
        if (inst->operand_count == 1 &&
            inst->op1.type == OP_REG)
        {
            int reg = reg_code(inst->op1.reg_name);

            output_buffer[0] = 0x58 + reg;

            return 1;
        }
    }

    /*
        MOV reg, imm32

        B8+r id
    */
    if (strcmp(inst->mnemonic, "MOV") == 0)
    {
        if (inst->operand_count == 2 &&
            inst->op1.type == OP_REG &&
            inst->op2.type == OP_IMM)
        {
            int reg = reg_code(inst->op1.reg_name);

            output_buffer[0] = 0xB8 + reg;

            write_imm32(
                &output_buffer[1],
                inst->op2.imm_value
            );

            return 5;
        }
    }

    /*
        MOV reg, reg

        89 /r
    */
    if (strcmp(inst->mnemonic, "MOV") == 0)
    {
        if (inst->operand_count == 2 &&
            inst->op1.type == OP_REG &&
            inst->op2.type == OP_REG)
        {
            int dst = reg_code(inst->op1.reg_name);
            int src = reg_code(inst->op2.reg_name);

            output_buffer[0] = 0x89;

            /*
                ModRM

                mod = 11
                reg = src
                rm  = dst
            */
            output_buffer[1] =
                0xC0 |
                (src << 3) |
                dst;

            return 2;
        }
    }

    printf(
        "[ENCODER] Instruccion no soportada: %s\n",
        inst->mnemonic
    );

    return 0;
}

static void write_imm32(
    uint8_t* buffer,
    int32_t value
)
{
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
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

static uint8_t make_modrm(
    uint8_t mod,
    uint8_t reg,
    uint8_t rm
)
{
    return (mod << 6) | (reg << 3) | rm;
}

static uint8_t make_sib(
    uint8_t scale,
    uint8_t index,
    uint8_t base
)
{
    return (scale << 6) | (index << 3) | base;
}

static void write_disp32(
    uint8_t* buffer,
    int32_t value
)
{
    buffer[0] = value & 0xFF;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = (value >> 16) & 0xFF;
    buffer[3] = (value >> 24) & 0xFF;
}

if (strcmp(inst->mnemonic, "ADD") == 0)
{
    if (inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);

        output_buffer[0] = 0x01;

        output_buffer[1] =
            make_modrm(
                3,
                src,
                dst
            );

        return 2;
    }
}

if (strcmp(inst->mnemonic, "SUB") == 0)
{
    if (inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);

        output_buffer[0] = 0x29;

        output_buffer[1] =
            make_modrm(
                3,
                src,
                dst
            );

        return 2;
    }
}

if (strcmp(inst->mnemonic, "CMP") == 0)
{
    if (inst->op1.type == OP_REG &&
        inst->op2.type == OP_REG)
    {
        int dst = reg_code(inst->op1.reg_name);
        int src = reg_code(inst->op2.reg_name);

        output_buffer[0] = 0x39;

        output_buffer[1] =
            make_modrm(
                3,
                src,
                dst
            );

        return 2;
    }
}

if (strcmp(inst->mnemonic, "JMP") == 0)
{
    output_buffer[0] = 0xE9;

    write_imm32(
        &output_buffer[1],
        inst->op1.imm_value
    );

    return 5;
}

if (strcmp(inst->mnemonic, "CALL") == 0)
{
    output_buffer[0] = 0xE8;

    write_imm32(
        &output_buffer[1],
        inst->op1.imm_value
    );

    return 5;
}

if (strcmp(inst->mnemonic, "JE") == 0)
{
    output_buffer[0] = 0x0F;
    output_buffer[1] = 0x84;

    write_imm32(
        &output_buffer[2],
        inst->op1.imm_value
    );

    return 6;
}

if (strcmp(inst->mnemonic, "JNE") == 0)
{
    output_buffer[0] = 0x0F;
    output_buffer[1] = 0x85;

    write_imm32(
        &output_buffer[2],
        inst->op1.imm_value
    );

    return 6;
}

if (strcmp(inst->mnemonic, "MOV") == 0)
{
    if (inst->op1.type == OP_REG &&
        inst->op2.type == OP_MEM)
    {
        int dst = reg_code(inst->op1.reg_name);

        int pos = 0;

        output_buffer[pos++] = 0x8B;

        if (inst->op2.has_base &&
            !inst->op2.has_index)
        {
            int base =
                reg_code(
                    inst->op2.base_reg
                );

            output_buffer[pos++] =
                make_modrm(
                    2,
                    dst,
                    base
                );

            write_disp32(
                &output_buffer[pos],
                inst->op2.displacement
            );

            pos += 4;

            return pos;
        }
    }
}

if (strcmp(inst->mnemonic, "MOV") == 0)
{
    if (inst->op1.type == OP_MEM &&
        inst->op2.type == OP_REG)
    {
        int src =
            reg_code(
                inst->op2.reg_name
            );

        int pos = 0;

        output_buffer[pos++] = 0x89;

        if (inst->op1.has_base &&
            !inst->op1.has_index)
        {
            int base =
                reg_code(
                    inst->op1.base_reg
                );

            output_buffer[pos++] =
                make_modrm(
                    2,
                    src,
                    base
                );

            write_disp32(
                &output_buffer[pos],
                inst->op1.displacement
            );

            pos += 4;

            return pos;
        }
    }
}