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
		640,480,0
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

	/*
	printf("%02x\n",romData[0x0100]);
	printf("%02x\n",romData[0x0101]);
	printf("%02x\n",romData[0x0102]);
	printf("%02x\n",romData[0x0103]);
	void (*handler)() = (void (*)())instructions[romData[0x0100]].handler;
	handler();
	*/

	return 0;
}
