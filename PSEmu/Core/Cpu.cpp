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
	next_instruction = read_u32();

	decode_and_execute(opcode);

	return cycles;
}

void Cpu::decode_and_execute(u32 opcode)
{
	u8 primary_opcode = (opcode >> 26) & 0x3F;

	InstructionBitField ibf;
	ibf.opcode = opcode;

	cycles = primary_lut[primary_opcode].cycles;
	primary_lut[primary_opcode].execute(ibf);
}

u32 Cpu::read_u32()
{
	u32 word = (bus->read_u8(pc + 1)) |
		(bus->read_u8(pc + 2) << 8) |
		(bus->read_u8(pc + 3) << 16) |
		(bus->read_u8(pc + 4) << 24);

	return word;
}

u32 Cpu::fetch_u32()
{
	u32 word = (bus->read_u8(pc++)) |
		(bus->read_u8(pc++) << 8) |
		(bus->read_u8(pc++) << 16) |
		(bus->read_u8(pc++) << 24);

	return word;
}

void Cpu::write_register(u8 register_index, u32 value)
{
	if (register_index > 0x1F) {
		printf("!!Register Write Error: Register index too large!!\n");
		return;
	}
	gprs[register_index] = value;
}

u32 Cpu::read_register(u8 register_index)
{
	if (register_index > 0x1F) {
		printf("!!Register Write Error: Register index too large!!\n");
		return 0xDEAD;
	}
	return gprs[register_index];
}

void Cpu::branch_delay_slot()
{
	decode_and_execute(next_instruction);
}

void Cpu::load_delay_slot()
{
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
	u8 rs = ibf.rs();
	u32 register_rs = read_register(rs);

	switch ((ibf.opcode >> 16) & 0x1F) {
		case 0b00000: //bltz
			if (register_rs < 0) branch(ibf);
		break;
		case 0b00001: //bgez 
			if (register_rs >= 0) branch(ibf);
		break;
		case 0b10000: { //bltzal
			write_register(RA, pc + 4); //place address following delay slot in ra
			if (register_rs < 0)
				branch(ibf);
		}
		break;
		case 0b10001: {//bgezal
			write_register(RA, pc + 4);
			if (register_rs >= 0)
				branch(ibf);
		}
		break;
	}
}

void Cpu::j(InstructionBitField& ibf)
{
	//align target address (shift left 2) to combine with upper 4 bits of PC
	u32 target = ibf.immediate_26();
	pc = (pc & 0xF0000000) | (target << 2);

	branch_delay_slot();
}

void Cpu::jal(InstructionBitField& ibf)
{
	u32 target = ibf.immediate_26();

	//store address (return address) of instruction after the delay slot
	//into the register ra
	write_register(RA, pc + 4); 
	
	pc = (pc & 0xF0000000) | (target << 2);

	branch_delay_slot();
}

void Cpu::branch(InstructionBitField& ibf)
{
	u32 imm = (s16)ibf.immediate_16();
	imm = sext_32(imm, 16);
	imm <<= 2;

	u32 delay_slot_address = pc;
	u32 target_address = delay_slot_address + imm;

	pc = target_address;
}

void Cpu::beq(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u8 rt = ibf.rt();

	u32 register_rs = read_register(rs);
	u32 register_rt = read_register(rt);

	if (register_rs == register_rt)
		branch(ibf);
}

void Cpu::bne(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u8 rt = ibf.rt();

	u32 register_rs = read_register(rs);
	u32 register_rt = read_register(rt);

	if (register_rs != register_rt)
		branch(ibf);
}

void Cpu::blez(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u32 register_rs = read_register(rs);

	if (register_rs <= 0)
		branch(ibf);
}

void Cpu::bgtz(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u32 register_rs = read_register(rs);

	if (register_rs > 0)
		branch(ibf);
}

void Cpu::addi(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);
	u32 result = register_rs + imm_se;

	write_register(rt, result);
	//trap on two's complement overflow
}

void Cpu::addiu(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);
	u32 result = register_rs + imm_se;

	write_register(rt, result);
}

void Cpu::slti(InstructionBitField& ibf)
{
	s32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	s32 register_rs = (s32)read_register(rs);

	u8 result = (register_rs < imm_se);

	write_register(rt, result);
}

void Cpu::sltiu(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);

	u8 result = (register_rs < imm_se);

	write_register(rt, result);
}

void Cpu::andi(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);
	u32 result = register_rs & imm;

	write_register(rt, result);
}

void Cpu::ori(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);
	u32 result = register_rs | imm;

	write_register(rt, result);
}

void Cpu::xori(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u8 rt = ibf.rt();
	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);
	u32 result = register_rs ^ imm;

	write_register(rt, result);
}

void Cpu::lui(InstructionBitField& ibf)
{
	u16 imm = ibf.immediate_16();
	u32 result = imm << 16;

	u8 rt = ibf.rt();

	write_register(rt, result);
}

void Cpu::jr(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u32 target = read_register(rs);

	pc = target;

	branch_delay_slot();
}

void Cpu::jalr(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u8 rd = ibf.rd();

	write_register(rd, pc + 4);

	u32 target = read_register(rs);
	pc = target;

	branch_delay_slot();
}
