#include "Cpu.h"

void Cpu::map_opcodes()
{
	primary_lut[0x00] = Instruction{ b(&Cpu::special), 1 };
	primary_lut[0x01] = Instruction{ b(&Cpu::bcond), 1 };
	primary_lut[0x02] = Instruction{ b(&Cpu::j), 1 };
	primary_lut[0x03] = Instruction{ b(&Cpu::jal), 1 };
	primary_lut[0x04] = Instruction{ b(&Cpu::beq), 1 };
	primary_lut[0x05] = Instruction{ b(&Cpu::bne), 1 };
	primary_lut[0x06] = Instruction{ b(&Cpu::blez), 1 };
	primary_lut[0x07] = Instruction{ b(&Cpu::bgtz), 1 };

	primary_lut[0x08] = Instruction{ b(&Cpu::addi), 1 };
	primary_lut[0x09] = Instruction{ b(&Cpu::addiu), 1 };
	primary_lut[0x0A] = Instruction{ b(&Cpu::slti), 1 };
	primary_lut[0x0B] = Instruction{ b(&Cpu::sltiu), 1 };
	primary_lut[0x0C] = Instruction{ b(&Cpu::andi), 1 };
	primary_lut[0x0D] = Instruction{ b(&Cpu::ori), 1 };
	primary_lut[0x0E] = Instruction{ b(&Cpu::xori), 1 };
	primary_lut[0x0F] = Instruction{ b(&Cpu::lui), 1 };

	primary_lut[0x20] = Instruction{ b(&Cpu::lb), 1 };
	primary_lut[0x21] = Instruction{ b(&Cpu::lh), 1 };
	primary_lut[0x22] = Instruction{ b(&Cpu::lwl), 1 };
	primary_lut[0x23] = Instruction{ b(&Cpu::lw), 1 };
	primary_lut[0x24] = Instruction{ b(&Cpu::lbu), 1 };
	primary_lut[0x25] = Instruction{ b(&Cpu::lhu), 1 };
	primary_lut[0x26] = Instruction{ b(&Cpu::lwr), 1 };
	primary_lut[0x27] = Instruction{ b(&Cpu::undefined_instruction), 1 };

	
	//secondary field
	secondary_lut[0x08] = Instruction{ b(&Cpu::jr), 1 };
	secondary_lut[0x09] = Instruction{ b(&Cpu::jalr), 1 };
}