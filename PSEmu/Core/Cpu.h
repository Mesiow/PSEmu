#pragma once
#include "Common.h"
#include <array>
#include <functional>

#define b(x) std::bind(x, this, std::placeholders::_1)

struct Bus;

struct InstructionBitField {
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
	u32 fetch_u32();

	void special(InstructionBitField& ibf); //secondary opcode
	void bcond(InstructionBitField& ibf);
	void j(InstructionBitField& ibf);
	void jal(InstructionBitField& ibf);
	void beq(InstructionBitField& ibf);
	void bne(InstructionBitField& ibf);
	void blez(InstructionBitField& ibf);
	void bgtz(InstructionBitField& ibf);
	

private:
	u32 gprs[0x20];
	u32 pc;
	u32 hi, lo; //mult/divide results

	std::array<Instruction, 0x3F> primary_lut;
	std::array<Instruction, 0x3F> secondary_lut;
	u8 cycles = 0;

	Bus* bus;
};