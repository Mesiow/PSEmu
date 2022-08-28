#include "Cpu.h"
#include "Bus.h"

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
	u32 opcode = fetch_u32();
	u8 primary_opcode = (opcode >> 26);

	InstructionBitField ibf;
	ibf.opcode = opcode;

	cycles = primary_lut[primary_opcode].cycles;
	primary_lut[primary_opcode].execute(ibf);

	return cycles;
}

u32 Cpu::fetch_u32()
{
	u32 word = (bus->read_u8(pc++)) |
		(bus->read_u8(pc++) << 8) |
		(bus->read_u8(pc++) << 16) |
		(bus->read_u8(pc++) << 24);

	return word;
}

void Cpu::set_register(u8 register_index, u32 value)
{
	if (register_index > 0x1F) {
		printf("!!Register Write Error: Register index too large!!\n");
		return;
	}
	gprs[register_index] = value;
}

u32 Cpu::get_register(u8 register_index)
{
	if (register_index > 0x1F) {
		printf("!!Register Write Error: Register index too large!!\n");
		return;
	}
	return gprs[register_index];
}

void Cpu::na_instruction(InstructionBitField& ibf)
{
	printf("!!N/A instruction!!");
	assert(false);
}

void Cpu::special(InstructionBitField& ibf)
{
	u8 secondary_opcode = (ibf.opcode & 0x3F);

	cycles = secondary_lut[secondary_opcode].cycles;
	secondary_lut[secondary_opcode].execute(ibf);
}

void Cpu::bcond(InstructionBitField& ibf)
{
}

void Cpu::j(InstructionBitField& ibf)
{
}

void Cpu::jal(InstructionBitField& ibf)
{
}

void Cpu::beq(InstructionBitField& ibf)
{
}

void Cpu::bne(InstructionBitField& ibf)
{
}

void Cpu::blez(InstructionBitField& ibf)
{
}

void Cpu::bgtz(InstructionBitField& ibf)
{
}

void Cpu::addi(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = get_register(rs);
	u32 result = register_rs + imm_se;

	set_register(rt, result);
	//trap on two's complement overflow
}

void Cpu::addiu(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = get_register(rs);
	u32 result = register_rs + imm_se;

	set_register(rt, result);
}

void Cpu::slti(InstructionBitField& ibf)
{
	s32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	s32 register_rs = (s32)get_register(rs);

	u8 result = (register_rs < imm_se);

	set_register(rt, result);
}

void Cpu::sltiu(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = get_register(rs);

	u8 result = (register_rs < imm_se);

	set_register(rt, result);
}

void Cpu::andi(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = get_register(rs);
	u32 result = register_rs & imm;

	set_register(rt, result);
}

void Cpu::ori(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = get_register(rs);
	u32 result = register_rs | imm;

	set_register(rt, result);
}

void Cpu::xori(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = get_register(rs);
	u32 result = register_rs ^ imm;

	set_register(rt, result);
}

void Cpu::lui(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u32 result = imm << 16;

	u8 rt = ibf.rt();

	set_register(rt, result);
}
