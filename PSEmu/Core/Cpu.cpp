#include "Cpu.h"

Cpu::Cpu()
{
	reset();
	map_opcodes();
}

Cpu::~Cpu()
{

}

void Cpu::reset()
{
	memset(gprs, 0x0, 0x20);
	pc = 0xBFC00000; //point to bios rom
	hi = lo = 0x0;
}

u8 Cpu::clock()
{
	u8 opcode = fetch_u32();
	u8 primary_opcode = (opcode >> 26);

	InstructionBitField ibf;
	ibf.opcode = opcode;

	cycles = primary_lut[primary_opcode].cycles;
	primary_lut[primary_opcode].execute(ibf);

	return cycles;
}

u32 Cpu::fetch_u32()
{
	return u32();
}

void Cpu::special(InstructionBitField& ibf)
{
	u8 secondary_opcode = (ibf.opcode & 0x3F);

	cycles = secondary_lut[secondary_opcode].cycles;
	secondary_lut[secondary_opcode].execute(ibf);
}
