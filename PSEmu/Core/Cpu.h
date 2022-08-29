#pragma once
#include "Common.h"
#include <array>
#include <functional>

#define b(x) std::bind(x, this, std::placeholders::_1)

struct Bus;

enum RegisterAlias : u8{
	RA = 31
};

struct DelaySlot {
	u32 instruction;
};

struct InstructionBitField {
	u8 rs();
	u8 rt();
	u8 rd();
	u16 immediate_16();
	u32 immediate_26();
	u32 opcode;
};

struct Instruction {
	std::function<void(InstructionBitField& ibf)> execute;
	u8 cycles;
};

struct Cpu {
	Cpu();
	~Cpu();

	void reset();
	void map_opcodes();

	u8 clock();
	void decode_and_execute(u32 opcode);

	u32 read_u32();
	u32 fetch_u32();

	void write_register(u8 register_index, u32 value);
	u32 read_register(u8 register_index);

	void branch_delay_slot();
	void load_delay_slot();

	void na_instruction(InstructionBitField& ibf);
	void special(InstructionBitField& ibf); //secondary opcode

	void bcond(InstructionBitField& ibf);
	void j(InstructionBitField& ibf);
	void jal(InstructionBitField& ibf);
	void beq(InstructionBitField& ibf);
	void bne(InstructionBitField& ibf);
	void blez(InstructionBitField& ibf);
	void bgtz(InstructionBitField& ibf);

	void addi(InstructionBitField& ibf);
	void addiu(InstructionBitField& ibf);
	void slti(InstructionBitField& ibf);
	void sltiu(InstructionBitField& ibf);
	void andi(InstructionBitField& ibf);
	void ori(InstructionBitField& ibf);
	void xori(InstructionBitField& ibf);
	void lui(InstructionBitField& ibf);

	//secondary
	void jr(InstructionBitField& ibf);
	void jalr(InstructionBitField& ibf);
	

private:
	u32 gprs[0x20];
	u32 pc;
	u32 hi, lo; //mult/divide results

	std::array<Instruction, 0x3F> primary_lut;
	std::array<Instruction, 0x3F> secondary_lut;
	u8 cycles = 0;
	u32 next_instruction = 0;

	Bus* bus;
};