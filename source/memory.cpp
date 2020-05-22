#include <iostream>
#include <random>
#include <fstream>
#include "memory.h"
#include "screen.h"
#include "cpu.h"
#include "input.h"

// Raw ROM binary
unsigned char romData[0x8000];
// Video RAM
unsigned char vram[0x2000];
// Switchable ROM bank
unsigned char sram[0x2000];
// Internal RAM
unsigned char wram[0x2000];
// Sprite Attrib Memory
unsigned char oam[0x100];
// I/O ports
unsigned char io[0x100];
// Internal RAM
unsigned char hram[0x80];

unsigned char ioReset[0x100] = {
	0x0F, 0x00, 0x7C, 0xFF, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,
	0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
	0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
	0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xC1, 0x00, 0xFE, 0xFF, 0xFF, 0xFF,
	0xF8, 0xFF, 0x00, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
	0x45, 0xEC, 0x52, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xFD, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
	0x0B, 0xF8, 0xC2, 0xCE, 0xF4, 0xF9, 0x0F, 0x7F, 0x45, 0x6D, 0x3D, 0xFE, 0x46, 0x97, 0x33, 0x5E,
	0x08, 0xEF, 0xF1, 0xFF, 0x86, 0x83, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0xB4, 0xB7, 0x06, 0xD5,
	0xD0, 0x7A, 0x00, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x1D, 0x77, 0x36, 0x75, 0x81, 0xAA, 0x70, 0x3A,
	0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
};

char *romTypeString[] = {
	"ROM_PLAIN",
	"ROM_MBC1",
	"ROM_MBC1",
	"ROM_MBC1_RAM_BATT",
	"ROM_MBC2",
	"ROM_MBC2_BATTERY",
	"ROM_RAM",
	"ROM_RAM_BATTERY",
	"ROM_MMM01",
	"ROM_MMM01_SRAM",
	"ROM_MMM01_SRAM_BATT",
	"ROM_MBC3_TIMER_BATT",
	"ROM_MBC3_TIMER_RAM_BATT",
	"ROM_MBC3",
	"ROM_MBC3_RAM",
	"ROM_MBC3_RAM_BATT",
	"ROM_MBC5",
	"ROM_MBC5_RAM",
	"ROM_MBC5_RAM_BATT",
	"ROM_MBC5_RUMBLE",
	"ROM_MBC5_RUMBLE_SRAM",
	"ROM_MBC5_RUMBLE_SRAM_BATT",
	"ROM_POCKET_CAMERA",
	"ROM_BANDAI_TAMA5",
	"ROM_HUDSON_HUC3",
	"ROM_HUDSON_HUC1"
};


int loadRom(char *romfilePath){
	size_t romfileSize;
	unsigned char romTitle[17];
	char romType;
	int i;
	
	std::ifstream romfileStream(romfilePath,std::ifstream::ate|std::ifstream::binary);
	romfileSize = static_cast<size_t>(romfileStream.tellg());
	romfileStream.seekg(0,std::ifstream::beg);

	for(i=0;i<romfileSize;i++){
		romfileStream.read((char *)&romData[i],sizeof(unsigned char));
	}

	for(i=0;i<16;i++){
		if(romData[ROM_OFFSET_NAME+i] == 0x80 || romData[ROM_OFFSET_NAME+i] == 0xc0) romTitle[i] = '\0';
		else romTitle[i] = romData[ROM_OFFSET_NAME+i];
	}

	romType = romData[ROM_OFFSET_TYPE];

	if(!romTypeString[romType]){
		printf("Unknown ROM Type: %#02x\n",romType);
		return -1;	
	}

	if(romType != ROM_PLAIN){
		printf("Only supporting 32KB games with no mappers\n");
		return -1;
	}

	printf("ROM Type: %s\n",romTypeString[romType]);
	printf("Title: %s\n",romTitle);
	romfileStream.close();
	return 0;
}


void copy(unsigned short destination,unsigned short source,size_t length){
	unsigned int i;
	for(i=0;i<length;i++){
		writeByte(destination+i,readByte(source+i));
	}
}


unsigned char readByte(unsigned short address){

	if(address <= 0x7fff){
		return romData[address];
	
	}else if(address >= 0x8000 && address <= 0x9fff){
		return vram[address - 0x8000];

	}else if(address >= 0xa000 && address <= 0xbfff){
		return sram[address - 0xa000];
	
	}else if(address >= 0xc000 && address <= 0xdfff){
		return wram[address - 0xc000];
	
	}else if(address >= 0xe000 && address <= 0xfdff){
		return wram[address - 0xe000];

	}else if(address >= 0xfe00 && address <= 0xfeff){
		return oam[address - 0xfe00];
			
	}else if(address == 0xff04){
		return (unsigned char)std::rand();
	
	}else if(address == 0xff40){
		return gpu.control;
		
	}else if(address == 0xff42){
		return gpu.scrollY;

	}else if(address == 0xff43){
		return gpu.scrollX;
		
	}else if(address == 0xff44){
		return gpu.scanline;

	}else if(address == 0xff00){
		if(!(io[0x00] & 0x20)){
			return (unsigned char)(0xc0 | keys.keys1 | 0x10);
		}else if(!(io[0x00] & 0x10)){
			return (unsigned char)(0xc0 | keys.keys2 | 0x20);
		}else if(!io[0x00] & 0x30){
			return 0xff;
		}else{
			return 0;
		}
	}else if(address == 0xff0f){
		return interrupt.flags;
		
	}else if(address >= 0xff00 && address <= 0xff7f){
		return io[address - 0xff00];

	}else if(address >= 0xff80 && address <= 0xfffe){
		return hram[address - 0xff80];		

	}else if(address == 0xffff){
		return interrupt.enable;
	}

	return 0;
}


unsigned short readShort(unsigned short address){
	return (readByte(address+1) << 8) | readByte(address);
	//return readByte(address) | (readByte(address+1) << 8);
}


unsigned short readShortFromStack(void){
	unsigned short value;
	value = readShort(regs.sp);
	//printf("readShortFromStack: regs.sp: %04x, value: %04x\n",regs.sp,value);
	regs.sp += 2;
	return value;
}


void writeByte(unsigned short address,unsigned char value){
	
	if(address >= 0xa000 && address <= 0xbfff){
		sram[address - 0xa000] = value;
	
	}else if(address >= 0x8000 && address <= 0x9fff){
		vram[address - 0x8000] = value;
		if(address <= 0x97ff)
			updateTile(address,value);
	
	}else if(address >= 0xc000 && address <= 0xdfff){
		wram[address - 0xc000] = value;
	
	}else if(address >= 0xe000 && address <= 0xfdff){
		wram[address - 0xe000] = value;
	
	}else if(address >= 0xfe00 && address <= 0xfeff){
		oam[address - 0xfe00] = value;
	
	}else if(address >= 0xff80 && address <= 0xfffe){
		hram[address - 0xff80] = value;
	
	}else if(address == 0xff40){
		gpu.control = value;	
	
	}else if(address == 0xff42){
		gpu.scrollY = value;
	
	}else if(address == 0xff43){
		gpu.scrollX = value;
	
	}else if(address == 0xff46){
		copy(0xfe00,value<<8,160);
	
	}else if(address == 0xff47){
		for(int i=0;i<4;i++)
			backgroundPalette[i] = palette[(value >> (i*2))&3];
	
	}else if(address == 0xff48){
		for(int i=0;i<4;i++)
			spritePalette[0][i] = palette[(value >> (i*2))&3];
	
	}else if(address == 0xff49){
		for(int i=0;i<4;i++)
			spritePalette[1][i] = palette[(value >> (i*2))&3];
	
	}else if(address >= 0xff00 && address <= 0xff7f){
		io[address - 0xff00] = value;

	}else if(address == 0xff0f){
		interrupt.flags = value;
	
	}else if(address == 0xffff){
		interrupt.enable = value;

	}
}

	
void writeShort(unsigned short address, unsigned short value) {
	writeByte(address, (unsigned char)(value & 0x00ff));
	writeByte(address + 1, (unsigned char)((value & 0xff00) >> 8));
}


void writeShortToStack(unsigned short value) {
	regs.sp -= 2;
	writeShort(regs.sp, value);
	//printf("writeShortToStack: regs.sp: %04x, value: %04x\n",regs.sp,value);
}
