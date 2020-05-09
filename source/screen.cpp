#include <iostream>
#include <SDL2/SDL.h>
#include "screen.h"
#include "memory.h"

unsigned char tiles[384][8][8];
struct rgb backgroundPalette[4];
struct rgb spritePalette[2][4];
struct gpu gpu;

const struct rgb palette[4] = {
	{255,255,255},
	{192,192,192},
	{96,96,96},
	{0,0,0},
};


void updateTile(unsigned short address, unsigned char value) {
	address &= 0x1ffe;
	
	unsigned short tile = (address >> 4) & 511;
	unsigned short y = (address >> 1) & 7;
	
	unsigned char x, bitIndex;
	for(x = 0; x < 8; x++) {
		bitIndex = 1 << (7 - x);
		tiles[tile][y][x] = ((vram[address] & bitIndex) ? 1 : 0) + ((vram[address + 1] & bitIndex) ? 2 : 0);
	}
}
