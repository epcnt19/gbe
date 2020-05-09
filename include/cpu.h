#ifndef _INC_CPU
#define _INC_CPU

struct registers {
	struct {
		union {
			struct {
				unsigned char f;
				unsigned char a;
			};
			unsigned short af;
		};
	};
	struct {
		union {
			struct {
				unsigned char c;
				unsigned char b;
			};
			unsigned short bc;
		};
	};
	struct {
		union {
			struct {
				unsigned char e;
				unsigned char d;
			};
			unsigned short de;
		};
	};
	struct {
		union {
			struct {
				unsigned char l;
				unsigned char h;
			};
			unsigned short hl;
		};
	};
	unsigned short sp;
	unsigned short pc;
};

struct interrupt {
	unsigned char master;
	unsigned char enable;
	unsigned char flags;
};

struct instruction {
	char *disassembly;
	unsigned char openrandlength;
	void *handler;
};

extern void cpuEmulation(void);
extern void reset(void);
extern void nop(void);
extern struct registers regs;
extern struct interrupt interrupt;
extern struct instruction instructions[];
#endif
