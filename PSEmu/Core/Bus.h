#pragma once
#include "Common.h"
#include <fstream>

#define BIOS_SIZE 0x80000
#define MAIN_RAM_SIZE 0x200000
#define ER_1_SIZE 0x800000
#define SCRATCHPAD_SIZE 0x1000
#define IO_PORTS_SIZE 0x2000

struct Bus {
	Bus();
	~Bus();
	void reset();
	void load_bios(const std::string& path);
	void write_u8(u32 address, u8 byte);
	u8 read_u8(u32 address);

private:
	u8* main_ram;
	u8* expansion_region_1;
	u8* scratchpad;
	u8* io_ports;
	u8* bios_rom;
};