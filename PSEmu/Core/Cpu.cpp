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
	memset(lds.output_gprs, 0x0, 0x20);

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

	if(load_delay)
		lds.save_output(); 

	cycles = primary_lut[primary_opcode].cycles;
	primary_lut[primary_opcode].execute(ibf);

	if (load_delay) {
		write_register(lds.register_index, lds.register_value);
		load_delay = false;
	}
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
	gprs[0x0] = 0x0;
}

u32 Cpu::read_register(u8 register_index)
{
	if (register_index > 0x1F) {
		printf("!!Register Write Error: Register index too large!!\n");
		return 0xDEAD;
	}
	return gprs[register_index];
}

void Cpu::handle_branch_delay_slot()
{
	decode_and_execute(next_instruction);
}

void Cpu::handle_load_delay_slot(u8 register_index, u32 value)
{
	//save target values of the load
	lds.register_index = register_index;
	lds.register_value = value;

	load_delay = true;
}

void LoadDelaySlot::save_output()
{
	output_gprs[register_index] = register_value;
}

void Cpu::undefined_instruction(InstructionBitField& ibf)
{
	printf("!!Undefined N/A instruction!!");
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

	handle_branch_delay_slot();
}

void Cpu::jal(InstructionBitField& ibf)
{
	u32 target = ibf.immediate_26();

	//store address (return address) of instruction after the delay slot
	//into the register ra
	write_register(RA, pc + 4); 
	
	pc = (pc & 0xF0000000) | (target << 2);

	handle_branch_delay_slot();
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

u32 Cpu::calculate_load_store_target_address(InstructionBitField& ibf)
{
	u32 imm_se = (s16)ibf.immediate_16();
	imm_se = sext_32(imm_se, 16);

	u8 rs = ibf.rs();

	u32 register_rs = read_register(rs);
	u32 target_address = register_rs + imm_se;

	return target_address;
}

void Cpu::load(InstructionBitField& ibf)
{
	switch ((ibf.opcode >> 26) & 0x7) {
		case 0b000: lb(ibf); break;
		case 0b001: lh(ibf); break;
		case 0b010: lwl(ibf); break;
		case 0b011: lw(ibf); break;
		case 0b100: lbu(ibf); break;
		case 0b101: lhu(ibf); break;
		case 0b110: lwr(ibf); break;
	}
}

void Cpu::lb(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);

	//read byte and sign extend it
	u32 byte = (s8)bus->read_u8(target_address);
	byte = sext_32(byte, 8);

	handle_load_delay_slot(ibf.rt(), byte);
}

void Cpu::lbu(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);

	//read byte and zero extend
	u32 byte = bus->read_u8(target_address);
	
	handle_load_delay_slot(ibf.rt(), byte);
}

void Cpu::lh(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);

	u32 halfword = (s16)bus->read_u16(target_address);
	halfword = sext_32(halfword, 16);

	handle_load_delay_slot(ibf.rt(), halfword);
}

void Cpu::lhu(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 halfword = bus->read_u16(target_address);

	handle_load_delay_slot(ibf.rt(), halfword);
}

void Cpu::lw(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 word = bus->read_u32(target_address);

	handle_load_delay_slot(ibf.rt(), word);
}

void Cpu::lwl(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	//force align address
	u32 aligned_address = target_address & 0xFFFFFFFC;

	u32 word = bus->read_u32(aligned_address);
	u32 register_rt = read_register(ibf.rt());

	//determine value based on alignment
	u32 loaded_value = word;
	switch (target_address & 0x3) {
		case 0b00: loaded_value = ((register_rt & 0x00FFFFFF) | (word << 24)); break;
		case 0b01: loaded_value = ((register_rt & 0x0000FFFF) | (word << 16)); break;
		case 0b10: loaded_value = ((register_rt & 0x000000FF) | (word << 8)); break;
		case 0b11: loaded_value = (register_rt = word); break;
	}

	handle_load_delay_slot(ibf.rt(), loaded_value);
}

void Cpu::lwr(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 aligned_address = target_address & 0xFFFFFFFC;

	u32 word = bus->read_u32(target_address);
	u32 register_rt = read_register(ibf.rt());

	u32 loaded_value = word;
	switch (target_address & 0x3) {
		case 0b00: loaded_value = (register_rt = word); break;
		case 0b01: loaded_value = ((register_rt & 0xFF000000) | (word >> 8)); break;
		case 0b10: loaded_value = ((register_rt & 0xFFFF0000) | (word >> 16)); break;
		case 0b11: loaded_value = ((register_rt & 0xFFFFFF00) | (word >> 24)); break;
	}

	handle_load_delay_slot(ibf.rt(), loaded_value);
}

void Cpu::store(InstructionBitField& ibf)
{
	switch ((ibf.opcode >> 26) & 0x7) {
		case 0b000: sb(ibf); break;
		case 0b001: sh(ibf); break;
		case 0b010: swl(ibf); break;
		case 0b011: sw(ibf); break;
		case 0b110: swr(ibf); break;
	}
}

void Cpu::sb(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 register_rt = read_register(ibf.rt());

	u8 byte = register_rt & 0xFF;
	bus->write_u8(target_address, byte);
}

void Cpu::sh(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 register_rt = read_register(ibf.rt());

	u16 halfword = register_rt & 0xFFFF;
	bus->write_u16(target_address, halfword);
}

void Cpu::swl(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 register_rt = read_register(ibf.rt());

	u32 target_address = calculate_load_store_target_address(ibf);
	//force align address
	u32 aligned_address = target_address & 0xFFFFFFFC;

	u32 register_rt = read_register(ibf.rt());
	//read current value at aligned address and update
	//the value with register rt
	u32 curr_mem_value = bus->read_u32(aligned_address);

	//determine value based on alignment
	u32 stored_value = 0;
	switch (target_address & 0x3) {
		case 0b00: stored_value = ((curr_mem_value & 0xFFFFFF00) | (register_rt >> 24)); break;
		case 0b01: stored_value = ((curr_mem_value & 0xFFFF0000) | (register_rt >> 16)); break;
		case 0b10: stored_value = ((curr_mem_value & 0xFF000000) | (register_rt >> 8)); break;
		case 0b11: stored_value = ((curr_mem_value & 0x00000000) | (register_rt >> 0)); break;
	}

	bus->write_u32(aligned_address, stored_value);
}

void Cpu::sw(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 register_rt = read_register(ibf.rt());

	u32 word = register_rt & 0xFFFFFFFF;
	bus->write_u32(target_address, word);
}

void Cpu::swr(InstructionBitField& ibf)
{
	u32 target_address = calculate_load_store_target_address(ibf);
	u32 register_rt = read_register(ibf.rt());

	u32 target_address = calculate_load_store_target_address(ibf);
	//force align address
	u32 aligned_address = target_address & 0xFFFFFFFC;

	u32 register_rt = read_register(ibf.rt());
	//read current value at aligned address and update
	//the value with register rt
	u32 curr_mem_value = bus->read_u32(aligned_address);

	//determine value based on alignment
	u32 stored_value = 0;
	switch (target_address & 0x3) {
		case 0b00: stored_value = ((curr_mem_value & 0x00000000) | (register_rt << 0)); break;
		case 0b01: stored_value = ((curr_mem_value & 0x000000FF) | (register_rt << 8)); break;
		case 0b10: stored_value = ((curr_mem_value & 0x0000FFFF) | (register_rt << 16)); break;
		case 0b11: stored_value = ((curr_mem_value & 0x00FFFFFF) | (register_rt << 24)); break;
	}

	bus->write_u32(aligned_address, stored_value);
}

void Cpu::jr(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u32 target = read_register(rs);

	pc = target;

	handle_branch_delay_slot();
}

void Cpu::jalr(InstructionBitField& ibf)
{
	u8 rs = ibf.rs();
	u8 rd = ibf.rd();

	write_register(rd, pc + 4);

	u32 target = read_register(rs);
	pc = target;

	handle_branch_delay_slot();
}
