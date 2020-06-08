#ifndef _INC_CPU
#define _INC_CPU

#define FLAGS_ZERO (1 << 7)
#define FLAGS_NEGATIVE (1 << 6)
#define FLAGS_HALFCARRY (1 << 5)
#define FLAGS_CARRY (1 << 4)

#define FLAGS_ISZERO (regs.f & FLAGS_ZERO)
#define FLAGS_ISNEGATIVE (regs.f & FLAGS_NEGATIVE)
#define FLAGS_ISCARRY (regs.f & FLAGS_CARRY)
#define FLAGS_ISHALFCARRY (regs.f & FLAGS_HALFCARRY)

#define FLAGS_ISSET(x) (regs.f & (x))
#define FLAGS_SET(x) (regs.f |= (x))
#define FLAGS_CLEAR(x) (regs.f &= ~(x))

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
extern unsigned long ticks;
extern struct registers regs;
extern struct interrupt interrupt;
extern struct instruction instructions[256];
#endif
