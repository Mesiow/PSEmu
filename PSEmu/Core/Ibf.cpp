#include "Cpu.h"

u16 InstructionBitField::immediate_16()
{
	return opcode & 0xFFFF;
}

u32 InstructionBitField::immediate_26()
{
	return opcode & 0x7FFFFFF;
}

u8 InstructionBitField::rs()
{
	return (opcode >> 21) & 0x1F;
}

u8 InstructionBitField::rt()
{
	return (opcode >> 16) & 0x1F;
}

u8 InstructionBitField::rd()
{
	return (opcode >> 11) & 0x3F;
}
