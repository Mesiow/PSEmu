#include "Bus.h"

Bus::Bus()
{
	reset();
}

Bus::~Bus()
{
	if (bios_rom != nullptr) delete bios_rom;
	if (main_ram != nullptr) delete main_ram;
	if (expansion_region_1 != nullptr) delete expansion_region_1;
	if (scratchpad != nullptr) delete scratchpad;
	if (io_ports != nullptr) delete io_ports;
}

void Bus::reset()
{
	main_ram = nullptr;
	expansion_region_1 = nullptr;
	scratchpad = nullptr;
	io_ports = nullptr;
	bios_rom = nullptr;
}

void Bus::load_bios(const std::string& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		u32 size = file.tellg();
		file.seekg(0, file.beg);

		if (size > BIOS_SIZE) {
			std::cerr << "Bios file: " + path + " is too large\n";
			return;
		}

		if (size == BIOS_SIZE) {
			printf("Original bios loaded\n");
			bios_rom = new u8[size];

			file.read((char*)bios_rom, BIOS_SIZE);
			file.close();
		}
		else {
			std::cerr << "!!Bios size does not match original size!!\n";
			return;
		}
	}
	else {
		std::cerr << "!!Bios file: " + path + "failed to open!!\n";
		return;
	}
}

void Bus::write_u8(u32 address, u8 byte)
{
	//KSEG1 for now
	if (address >= 0xA0000000 && address <= (0xA0000000 + MAIN_RAM_SIZE)) {
		main_ram[address - 0xA0000000] = byte;
	}
	//Open bus after main ram??? ignore
	else if (address > (0xA0000000 + MAIN_RAM_SIZE) && address < 0xBF000000) {
		printf("U8 Memory write attempt after main ram 2048K range (open bus)\n");
		return;
	}

	//Expansion region 1
	else if (address >= 0xBF000000 && address < (0xBF000000 + ER_1_SIZE)) {
		expansion_region_1[address - 0xBF000000] = byte;
	}
	else if (address >= 0xBF800000 && address < (0xBF800000 + SCRATCHPAD_SIZE)) {
		scratchpad[address - 0xBF800000] = byte;
	}
	else if (address >= 0xBF801000 && address <= (0xBF801000 + IO_PORTS_SIZE)) {
		io_ports[address - 0xBF801000] = byte;
	}
	else {
		printf("U8 Memory write attempt at unhandled address: 0x%08X\n", address);
		return;
	}
}

u8 Bus::read_u8(u32 address)
{
	if (address >= 0xA0000000 && address <= (0xA0000000 + MAIN_RAM_SIZE)) {
		return main_ram[address - 0xA0000000];
	}
	//Open bus after main ram??? ignore
	else if (address > (0xA0000000 + MAIN_RAM_SIZE) && address < 0xBF000000) {
		printf("U8 Memory read attempt after main ram 2048K range (open bus)\n");
		return 0xFF;
	}

	//Expansion region 1
	else if (address >= 0xBF000000 && address < (0xBF000000 + ER_1_SIZE)) {
		return expansion_region_1[address - 0xBF000000];
	}
	else if (address >= 0xBF800000 && address < (0xBF800000 + SCRATCHPAD_SIZE)) {
		return scratchpad[address - 0xBF800000];
	}
	else if (address >= 0xBF801000 && address <= (0xBF801000 + IO_PORTS_SIZE)) {
		return io_ports[address - 0xBF801000];
	}
	else {
		printf("U8 Memory read attempt at unhandled address: 0x%08X\n", address);
		return 0x00;
	}
}
