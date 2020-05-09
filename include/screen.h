#ifndef _INC_SCREEN
#define _INC_SCREEN

#include <iostream>
#include <SDL2/SDL.h>

struct rgb {
	unsigned char r,g,b;	
};

struct gpu {
	unsigned char control;
	unsigned char scrollX;
	unsigned char scrollY;
	unsigned char scanline;
	unsigned long tick;
};

extern unsigned char tiles[384][8][8];
extern struct rgb backgroundPalette[4];
extern struct rgb spritePalette[2][4];
extern const struct rgb palette[4];
extern struct gpu gpu;
extern void updateTile(unsigned short address, unsigned char value);

#endif
