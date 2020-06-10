#include <iostream>
#include <ctime>
#include <fstream>
#include <SDL2/SDL.h>
#include "screen.h"
#include "memory.h"
#include "input.h"
#include "cpu.h"

using namespace std;
char* romfilePath = "../rom/Tetris.gb";


void quit(void){
	// exit handler
	exit(0);
}


#define INTERRUPTS_VBLANK	(1 << 0)
#define INTERRUPTS_LCDSTAT	(1 << 1)
#define INTERRUPTS_TIMER	(1 << 2)
#define INTERRUPTS_SERIAL	(1 << 3)
#define INTERRUPTS_JOYPAD	(1 << 4)

enum gpuMode {
	GPU_MODE_HBLANK = 0,
	GPU_MODE_VBLAK = 1,
	GPU_MODE_OAM = 2,
	GPU_MODE_VRAM = 3
};

void gpuEmulation(void){
	static gpuMode gpuMode = GPU_MODE_HBLANK;	
	static int lastTicks = 0;
	gpu.tick += ticks - lastTicks;
	lastTicks = ticks;

	switch(gpuMode){
		case GPU_MODE_HBLANK:
			if(gpu.tick >= 204){
				gpu.scanline++;
				if(gpu.scanline == 143){
					if(interrupt.enable & INTERRUPTS_VBLANK)
						interrupt.flags |= INTERRUPTS_VBLANK;
					gpuMode = GPU_MODE_VBLAK;
				}else{
					gpuMode = GPU_MODE_OAM;
				}
				gpu.tick -= 204;
			}
			break;

		case GPU_MODE_VBLAK:
			if(gpu.tick >= 456){
				gpu.scanline++;
				if(gpu.scanline > 153){
					gpu.scanline = 0;
					gpuMode = GPU_MODE_OAM;
				}
				gpu.tick -= 456;
			}
			break;

		case GPU_MODE_OAM:
			if(gpu.tick >= 80){
				gpuMode = GPU_MODE_VRAM;
				gpu.tick -= 80;
			}
			break;

		case GPU_MODE_VRAM:
			if(gpu.tick >= 172){
				gpuMode = GPU_MODE_HBLANK;
				gpu.tick -= 172;
			}
			break;
	}
}


void vblank(void){
	/*
	glClear(GL_COLOR_BUFFER_BIT);
	glRasterPos2f(-1, 1);
	glPixelZoom(1, -1);
	glDrawPixels(160, 144, GL_RGB, GL_UNSIGNED_BYTE, framebuffer);
	*/
	interrupt.master = 0;
	writeShortToStack(regs.pc);
	regs.pc = 0x40;
	ticks += 12;
}


void lcdStat(void){
	interrupt.master = 0;
	writeShortToStack(regs.pc);
	regs.pc = 0x48;
	ticks += 12;
}


void timer(void){
	interrupt.master = 0;
	writeShortToStack(regs.pc);
	regs.pc = 0x50;
	ticks += 12;
}


void serial(void){
	interrupt.master = 0;
	writeShortToStack(regs.pc);
	regs.pc = 0x58;
	ticks += 12;
}


void joypad(void){
	interrupt.master = 0;
	writeShortToStack(regs.pc);
	regs.pc = 0x60;
	ticks += 12;
}


void returnFromInterrupt(void){
	unsigned short retaddr;
	interrupt.master = 1;
	retaddr = readShortFromStack();
	regs.pc = retaddr;
}


void interruptEmulation(void){
	//printf("interrupt.master: %d\n",interrupt.master);
	//printf("interrupt.enable: %d\n",interrupt.enable);
	//printf("interrupt.flags: %d\n",interrupt.flags);

	if(interrupt.master && interrupt.enable && interrupt.flags){
		unsigned char fire  = interrupt.enable & interrupt.flags;

		if(fire & INTERRUPTS_VBLANK){
			interrupt.flags &= ~INTERRUPTS_VBLANK;
			printf("fired: INTERRUPTS_VBLANK\n");
			vblank();
		}

		if(fire & INTERRUPTS_LCDSTAT){
			interrupt.flags &= ~INTERRUPTS_LCDSTAT;
			printf("fired: INTERRUPTS_LCDSTAT\n");
			lcdStat();
		}

		if(fire & INTERRUPTS_TIMER){
			interrupt.flags &= ~INTERRUPTS_TIMER;
			printf("fired: INTERRUPTS_TIMER\n");
			timer();
		}

		if(fire & INTERRUPTS_SERIAL){
			interrupt.flags &= ~INTERRUPTS_SERIAL;
			printf("fired: INTERRUPTS_SERIAL\n");
			serial();
		}

		if(fire & INTERRUPTS_JOYPAD){
			interrupt.flags &= ~INTERRUPTS_JOYPAD;
			printf("fired: INTERRUPTS_JOYPAD\n");
			joypad();
		}
	}
}


int main(void){
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	bool exitFlag = false;

	if(SDL_Init(SDL_INIT_VIDEO) != NULL) {
		SDL_Quit();
		quit();
	}

	window = SDL_CreateWindow(
		"gbe",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		160,144,0
	);

	if(window == NULL){
		SDL_Quit();
		quit();
	}else{
		renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
	}

	std::srand(std::time(NULL));

	if(loadRom(romfilePath) == -1){
		printf("Failed loading ROM\n");
		quit();
	}

	reset();

	// for debug
	int count_done = 0;
	for(int i=0;i<256;i++)
		if(instructions[i].handler != NULL)
			count_done++;		
	printf("current number of cpu instructions: %d/256\n",count_done);
	
	while(1){
		SDL_Event keyEvent;
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);

		if(SDL_PollEvent(&keyEvent)){
			switch(keyEvent.type){
				case SDL_QUIT:
					printf("SDL_QUIT\n");
					exitFlag = true;
					break;
				case SDL_KEYDOWN:
					printf("SDL_KEYDOWN && ");
					switch(keyEvent.key.keysym.sym){
						case SDLK_UP:
							printf("SDLK_UP: up key\n");
							keys.up = true;
							break;
						case SDLK_DOWN:
							printf("SDLK_DOWN: down key\n");
							keys.down = true;
							break;
						case SDLK_RIGHT:
							printf("SDLK_RIGHT: right key\n");
							keys.right = true;
							break;
						case SDLK_LEFT:
							printf("SDLK_LEFT: left key\n");
							keys.left = true;
							break;
						case SDLK_w:
							printf("SDLK_w: a key\n");
							keys.a = true;
							break;
						case SDLK_q:
							printf("SDLK_q: b key\n");
							keys.b = true;
							break;
						case SDLK_s:
							printf("SDLK_s: start key\n");
							keys.start = true;
							break;
						case SDLK_a:
							printf("SDLK_a: select key\n");
							keys.select = true;
							break;
						default:
							printf("other\n");
							break;			
					}
					exitFlag = false;
					break;
			}
		}

		if(exitFlag){
			break;
		}
	
		cpuEmulation();
		gpuEmulation();
		interruptEmulation();

		keys.a = false;
		keys.b = false;
		keys.start = false;
		keys.select = false;
		keys.up = false;
		keys.down = false;
		keys.left = false;
		keys.right = false;

	}
	
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
