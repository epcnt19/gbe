#ifndef _INC_PREFIX_CB
#define _INC_PREFIX_CB

struct cbInstruction {
	char *disassembly;
	void *handler;
};

extern struct cbInstruction cbInstructions[256];
extern void cbEmulation(unsigned char opcode);

#endif
