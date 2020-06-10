#include<iostream>
#include "cpu.h"
#include "memory.h"
#include "screen.h"
#include "input.h"
#include "prefix_cb.h"

//how to use: regs.af = 0xdead;
struct registers regs;
struct interrupt interrupt;
unsigned long ticks;
bool break_point = false;


unsigned char calc_inc(unsigned char value){
	if(value & 0x0f){
		FLAGS_CLEAR(FLAGS_HALFCARRY);
	}else{
		FLAGS_SET(FLAGS_HALFCARRY);	
	}

	value++;

	if(value){
		FLAGS_CLEAR(FLAGS_ZERO);
	}else{
		FLAGS_SET(FLAGS_ZERO);
	}

	FLAGS_SET(FLAGS_NEGATIVE);
	return value;
}


unsigned char calc_dec(unsigned char value){
	if(value & 0x0f){
		FLAGS_CLEAR(FLAGS_HALFCARRY);
	}else{
		FLAGS_SET(FLAGS_HALFCARRY);	
	}

	value--;

	if(value){
		FLAGS_CLEAR(FLAGS_ZERO);
	}else{
		FLAGS_SET(FLAGS_ZERO);
	}

	FLAGS_SET(FLAGS_NEGATIVE);
	return value;
}


void calc_add(unsigned char *dst_value,unsigned char src_value){
	unsigned int result;
	result = *dst_value + src_value;
	
	// checking full-carry (overflow)
	if(result & 0xff00)
		FLAGS_SET(FLAGS_CARRY);
	else
		FLAGS_CLEAR(FLAGS_CARRY);

	// get result (1 byte)
	*dst_value = (unsigned char)(0xff00 & result);

	// checking zero
	if(*dst_value)
		FLAGS_CLEAR(FLAGS_ZERO);
	else
		FLAGS_SET(FLAGS_ZERO);

	// check half-carry
	if(((*dst_value & 0x0f) + (src_value & 0x0f)) > 0x0f)
		FLAGS_SET(FLAGS_HALFCARRY);
	else
		FLAGS_CLEAR(FLAGS_HALFCARRY);

	FLAGS_CLEAR(FLAGS_NEGATIVE);
}


void calc_sub(unsigned char value){
	
	// checking full-carry
	if(value > regs.a)
		FLAGS_SET(FLAGS_CARRY);
	else
		FLAGS_CLEAR(FLAGS_CARRY);
	
	// checking half-carry
	if((value & 0x0f) > (regs.a & 0x0f))
		FLAGS_SET(FLAGS_HALFCARRY);
	else
		FLAGS_CLEAR(FLAGS_HALFCARRY);

	regs.a -= value;

	// checking zero
	if(regs.a)
		FLAGS_CLEAR(FLAGS_ZERO);
	else
		FLAGS_SET(FLAGS_ZERO);
	
	FLAGS_SET(FLAGS_NEGATIVE);
}


void calc_and(unsigned char value){
	regs.a &= value;
	
	if(regs.a){
		FLAGS_CLEAR(FLAGS_ZERO);
	}else{
		FLAGS_SET(FLAGS_ZERO);
	
	}

	FLAGS_CLEAR(FLAGS_NEGATIVE | FLAGS_CARRY);
	FLAGS_SET(FLAGS_HALFCARRY);
}


void calc_or(unsigned char value){
	regs.a |= value;

	if(regs.a){
		FLAGS_CLEAR(FLAGS_ZERO);
	}else{
		FLAGS_SET(FLAGS_ZERO);
	}

	FLAGS_CLEAR(FLAGS_NEGATIVE | FLAGS_HALFCARRY | FLAGS_CARRY);
}


void calc_xor(unsigned char value){
	regs.a ^= value;

	if(regs.a){
		FLAGS_CLEAR(FLAGS_ZERO);
	}else{
		FLAGS_SET(FLAGS_ZERO);
	}

	FLAGS_CLEAR(FLAGS_NEGATIVE | FLAGS_HALFCARRY | FLAGS_CARRY);
}


// 0x00
void nop(void){}

// 0x01
void ld_bc_nn(unsigned short operand){
	regs.bc = operand;
}

// 0x05
void dec_b(void){
	regs.b = calc_dec(regs.b);
}

// 0x06
void ld_b_n(unsigned char operand){
	regs.b = operand;
}

// 0x0b
void dec_bc(void){
	regs.bc--;
}

// 0x0c
void inc_c(void){
	regs.c = calc_inc(regs.c);
}

// 0x0d
void dec_c(void){
	regs.c = calc_dec(regs.c);
}

// 0x0e
void ld_c_n(unsigned char operand){
	regs.c = operand;
}

// 0x20
void jr_nz_n(unsigned char operand){
	if(FLAGS_ISZERO){
		ticks += 8;
	}else{
		regs.pc += (signed char)operand;
		ticks += 12;
	}
}

// 0x21
void ld_hl_nn(unsigned short operand){
	regs.hl = operand;
}

// 0x28
void jr_z_n(unsigned char operand){
	if(FLAGS_ISZERO){
		regs.pc += (signed char)operand;
		ticks += 12;
	}else{
		ticks += 8;
	}
}

// 0x2a
void ldi_a_hl(void){
	regs.a = readByte(regs.hl);
	regs.hl++;
}

// 0x2f
void cpl(void){
	regs.a = ~regs.a;
	FLAGS_SET(FLAGS_NEGATIVE|FLAGS_HALFCARRY);
}

// 0x31
void ld_sp_nn(unsigned short operand){
	regs.sp = operand;
}

// 0x32
void ldd_hl_a(void){
	writeByte(regs.hl,regs.a);
	regs.hl--;
}

// 0x34
void inc_hl(void){
	writeByte(regs.hl,calc_inc(readByte(regs.hl)));
}

// 0x36
void ld_hl_n(unsigned char operand){
	writeByte(regs.hl,operand);
}

// 0x3c
void inc_a(void){
	regs.a = calc_inc(regs.a);
}

// 0x3d
void dec_a(void){
	regs.a = calc_dec(regs.a);
}

// 0x3e
void ld_a_n(unsigned char operand){
	regs.a = operand;
}

// 0x40: nop

// 0x41
void ld_b_c(void){
	regs.b = regs.c;
}

// 0x42
void ld_b_d(void){
	regs.b = regs.d;
}

// 0x43
void ld_b_e(void){
	regs.b = regs.e;
}

// 0x44
void ld_b_h(void){
	regs.b = regs.h;
}

// 0x45
void ld_b_l(void){
	regs.b = regs.l;
}

// 0x46
void ld_b_hl(void){
	regs.b = readByte(regs.hl);
}

// 0x47
void ld_b_a(void){
	regs.b = regs.a;
}

// 0x48
void ld_c_b(void){
	regs.c = regs.b;
}

// 0x49: nop

// 0x4a
void ld_c_d(void){
	regs.c = regs.d;
}

// 0x4b
void ld_c_e(void){
	regs.c = regs.e;
}

// 0x4c
void ld_c_h(void){
	regs.c = regs.h;
}

// 0x4d
void ld_c_l(void){
	regs.c = regs.l;
}

// 0x4e
void ld_c_hl(void){
	regs.c = readByte(regs.hl);
}

// 0x4f
void ld_c_a(void){
	regs.c = regs.a;
}

// 0x78
void ld_a_b(void){
	regs.a = regs.b;
}

// 0x79
void ld_a_c(void){
	regs.a = regs.c;
}

// 0x7a
void ld_a_d(void){
	regs.a = regs.d;
}

// 0x7b
void ld_a_e(void){
	regs.a = regs.e;
}

// 0x7c
void ld_a_h(void){
	regs.a = regs.h;
}

// 0x7d
void ld_a_l(void){
	regs.a = regs.l;
}

// 0x7e
void ld_a_hl(void){
	regs.a = readByte(regs.hl);
}

// 0x7f: nop

//0x80
void add_a_b(void){
	calc_add(&regs.a,regs.b);
}

//0x81
void add_a_c(void){
	calc_add(&regs.a,regs.c);
}

//0x82
void add_a_d(void){
	calc_add(&regs.a,regs.d);
}

//0x83
void add_a_e(void){
	calc_add(&regs.a,regs.e);
}

//0x84
void add_a_h(void){
	calc_add(&regs.a,regs.h);
}

//0x85
void add_a_l(void){
	calc_add(&regs.a,regs.l);
}

//0x86
void add_a_hl(void){
	calc_add(&regs.a,readByte(regs.hl));
}

//0x87
void add_a_a(void){
	calc_add(&regs.a,regs.a);
}

//0x90
void sub_a_b(void){
	calc_sub(regs.b);
}

//0x91
void sub_a_c(void){
	calc_sub(regs.c);
}

//0x92
void sub_a_d(void){
	calc_sub(regs.d);
}

//0x93
void sub_a_e(void){
	calc_sub(regs.e);
}

//0x94
void sub_a_h(void){
	calc_sub(regs.h);
}

//0x95
void sub_a_l(void){
	calc_sub(regs.l);
}

//0x96
void sub_a_hl(void){
	calc_sub(readByte(regs.hl));
}

//0x97
void sub_a_a(void){
	calc_sub(regs.a);
}

// 0xa0
void and_b(void){
	calc_and(regs.b);
}

// 0xa1
void and_c(void){
	calc_and(regs.c);
}

// 0xa2
void and_d(void){
	calc_and(regs.d);
}

// 0xa3
void and_e(void){
	calc_and(regs.e);
}

// 0xa4
void and_h(void){
	calc_and(regs.h);
}

// 0xa5
void and_l(void){
	calc_and(regs.l);
}

// 0xa6
void and_hl(void){
	calc_and(readByte(regs.hl));
}

// 0xa7
void and_a(void){
	calc_and(regs.a);
}

// 0xa8
void xor_b(void){
	calc_xor(regs.b);
}

// 0xa9
void xor_c(void){
	calc_xor(regs.c);
}

// 0xaa
void xor_d(void){
	calc_xor(regs.d);
}

// 0xab
void xor_e(void){
	calc_xor(regs.e);
}

// 0xac
void xor_h(void){
	calc_xor(regs.h);
}

// 0xad
void xor_l(void){
	calc_xor(regs.l);
}

// 0xae
void xor_hl(void){
	calc_xor(readByte(regs.hl));
}

// 0xaf
void xor_a(void){
	calc_xor(regs.a);
}

// 0xb0
void or_b(void){
	calc_or(regs.b);
}

// 0xb1
void or_c(void){
	calc_or(regs.c);
}

// 0xb2
void or_d(void){
	calc_or(regs.d);
}

// 0xb3
void or_e(void){
	calc_or(regs.e);
}

// 0xb4
void or_h(void){
	calc_or(regs.h);
}

// 0xb5
void or_l(void){
	calc_or(regs.l);
}

// 0xb6
void or_hl(void){
	calc_or(readByte(regs.hl));
}

// 0xb7
void or_a(void){
	calc_or(regs.a);
}

// 0xc0
void ret_nz(void){
	if(FLAGS_ISZERO){
		ticks += 0;
	}else{
		regs.pc = readShortFromStack();
		ticks += 20;
	}
}

// 0xc1
void pop_bc(void){
	regs.bc = readShortFromStack();		
}

// 0xc3
void jp_nn(unsigned short operand){
	regs.pc = operand;
}

// 0xc8
void ret_z(void){
	if(FLAGS_ISZERO){
		regs.pc = readShortFromStack();
		ticks += 20;
	}else{
		ticks += 8;
	}
}

// 0xc9
void ret(void){
	unsigned short retaddr;
	retaddr = readShortFromStack();
	regs.pc = retaddr;
	//break_point = true;
}

// 0xcd
void call_nn(unsigned short operand){
	writeShortToStack(regs.pc);
	regs.pc = operand;
}

// 0xc5
void push_bc(void){
	writeShortToStack(regs.bc);
}

// 0xcf
void rst_8(void){
	writeShortToStack(regs.pc);
	regs.pc = 0x0008;
}

// 0xd1
void pop_de(void){
	regs.de = readShortFromStack();		
}

// 0xd5
void push_de(void){
	writeShortToStack(regs.de);
}

// 0xd9
void reti(void){
	interrupt.master = 1;
	regs.pc = readShortFromStack();
}

// 0xdf
void rst_18(void){
	writeShortToStack(regs.pc);
	regs.pc = 0x0018;
}

// 0xe0
void ldh_n_a(unsigned char operand){
	writeByte(0xff00 + operand,regs.a);
}

// 0xe1
void pop_hl(void){
	regs.hl = readShortFromStack();		
}

// 0xe2
void ldh_c_a(void){
	writeByte(0xff00 + regs.c,regs.a);
}

// 0xe5
void push_hl(void){
	writeShortToStack(regs.hl);
}

// 0xe6
void and_n(unsigned char operand){
	regs.a &= operand;
	FLAGS_CLEAR(FLAGS_CARRY|FLAGS_NEGATIVE);
	FLAGS_SET(FLAGS_HALFCARRY);
	if(regs.a){
		FLAGS_CLEAR(FLAGS_ZERO);
	}else{
		FLAGS_SET(FLAGS_ZERO);
	}
}

// 0xea
void ld_nn_a(unsigned short operand){
	writeByte(operand,regs.a);
}

// 0xef
void rst_28(void){
	writeShortToStack(regs.pc);
	regs.pc = 0x0028;
}

// 0xf0
void ldh_a_n(unsigned char operand){
	regs.a = readByte(0xff00 + operand);
}

// 0xf1
void pop_af(void){
	regs.af = readShortFromStack();
}

// 0xfa
void ld_a_nn(unsigned short operand){
	regs.a = readByte(operand);
}

// 0xfb
void ei(void){
	interrupt.master = 1;
}

// 0xf3
void di(void){
	interrupt.master = 0;
}

// 0xf5
void push_af(void){
	writeShortToStack(regs.af);
}

// 0xfe
void cp_n(unsigned char operand){
	FLAGS_SET(FLAGS_NEGATIVE);

	if(regs.a == operand)
		FLAGS_SET(FLAGS_ZERO);
	else
		FLAGS_CLEAR(FLAGS_ZERO);

	if((regs.a & 0x0f) < (operand & 0x0f))
		FLAGS_SET(FLAGS_HALFCARRY);
	else
		FLAGS_SET(FLAGS_HALFCARRY);

	if(regs.a < operand)
		FLAGS_SET(FLAGS_CARRY);
	else
		FLAGS_CLEAR(FLAGS_CARRY);
}

// 0xff
void rst_38(void){
	writeShortToStack(regs.pc);
	regs.pc = 0x0038;
}


struct instruction instructions[256] = {
	{"NOP",0,(void *)&nop},			//0x00
	{"LD BC, 0x%04X",2,(void *)&ld_bc_nn},		//0x01
	{"LD (BC), A",0,NULL},			//0x02
	{"INC BC",0,NULL},				//0x03
	{"INC B",0,NULL},				//0x04
	{"DEC B",0,(void *)&dec_b},				//0x05
	{"LD B, 0x%02x",1,(void *)&ld_b_n},		//0x06
	{"RLCA",0,NULL},				//0x07
	{"LD (0x%04x), SP",2,NULL},		//0x08
	{"ADD HL, BC",0,NULL},			//0x09
	{"LD A, (BC)",0,NULL},			//0x0a
	{"DEC BC",0,(void *)&dec_bc},				//0x0b
	{"INC C",0,(void *)&inc_c},				//0x0c
	{"DEC C",0,(void *)&dec_c},				//0x0d
	{"LD C, 0x%02x",1,(void *)&ld_c_n},		//0x0e
	{"RRCA",0,NULL},				//0x0f
	{"STOP",1,NULL},				//0x10
	{"LD DE, 0x%04x",2,NULL},		//0x11
	{"LD (DE), A",0,NULL},			//0x12
	{"INC DE",0,NULL},				//0x13
	{"INC D",0,NULL},				//0x14
	{"DEC D",0,NULL},				//0x15
	{"LD D, 0x%02x",1,NULL},		//0x16
	{"RLA",0,NULL},					//0x17
	{"JR 0x%02x",1,NULL},			//0x18
	{"ADD HL, DE",0,NULL},			//0x19
	{"LD A, (DE)",0,NULL},			//0x1a
	{"DEC DE",0,NULL},				//0x1b
	{"INC E",0,NULL},				//0x1c
	{"DEC E",0,NULL},				//0x1d
	{"LD E, 0x%02x",1,NULL},		//0x1e
	{"RRA",0,NULL},					//0x1f
	{"JR NZ, 0x%02x",1,(void *)&jr_nz_n},		//0x20
	{"LD HL, 0x%04x",2,(void *)&ld_hl_nn},		//0x21
	{"LDI (HL), A",0,NULL},			//0x22
	{"INC HL",0,NULL},				//0x23
	{"INC H",0,NULL},				//0x24
	{"DEC H",0,NULL},				//0x25
	{"LD H, 0x%02x",1,NULL},		//0x26
	{"DAA",0,NULL},					//0x27
	{"JR Z, 0x%02x",1,(void *)jr_z_n},		//0x28
	{"ADD HL, HL",0,NULL},			//0x29
	{"LDI A, (HL)",0,(void *)&ldi_a_hl},			//0x2a
	{"DEC HL",0,NULL},				//0x2b
	{"INC L",0,NULL},				//0x2c
	{"DEC L",0,NULL},				//0x2d
	{"LD L, 0x%02x",1,NULL},		//0x2e
	{"CPL",0,(void *)&cpl},					//0x2f
	{"JR NC, 0x%02x",1,NULL},		//0x30
	{"LD SP, 0x%04x",2,(void *)&ld_sp_nn},		//0x31
	{"LDD (HL), A",0,(void *)&ldd_hl_a},			//0x32
	{"INC SP",0,NULL},				//0x33
	{"INC (HL)",0,(void *)&inc_hl},			//0x34
	{"DEC (HL)",0,NULL},			//0x35
	{"LD (HL), 0x%02x",1,(void *)&ld_hl_n},		//0x36
	{"CCF",0,NULL},					//0x37
	{"JR C, 0x%02x",1,NULL},		//0x38
	{"ADD HL, SP",0,NULL},			//0x39
	{"LDD A, (HL)",0,NULL},			//0x3a
	{"DEC SP",0,NULL},				//0x3b
	{"INC A",0,(void *)&inc_a},				//0x3c
	{"DEC A",0,(void *)&dec_a},				//0x3d
	{"LD A, 0x%02x",1,(void *)&ld_a_n},		//0x3e
	{"CCF",0,NULL},					//0x3f
	{"LD B, B",0,(void *)&nop},				//0x40
	{"LD B, C",0,(void *)&ld_b_c},				//0x41
	{"LD B, D",0,(void *)&ld_b_d},				//0x42
	{"LD B, E",0,(void *)&ld_b_e},				//0x43
	{"LD B, H",0,(void *)&ld_b_h},				//0x44
	{"LD B, L",0,(void *)&ld_b_l},				//0x45
	{"LD B, (HL)",0,(void *)&ld_b_hl},			//0x46
	{"LD B, A",0,(void *)&ld_b_a},				//0x47
	{"LD C, B",0,(void *)&ld_c_a},				//0x48
	{"LD C, C",0,(void *)&nop},				//0x49
	{"LD C, D",0,(void *)&ld_c_d},				//0x4a
	{"LD C, E",0,(void *)&ld_c_e},				//0x4b
	{"LD C, H",0,(void *)&ld_c_h},				//0x4c
	{"LD C, L",0,(void *)&ld_c_l},				//0x4d
	{"LD C, (HL)",0,(void *)&ld_c_hl},			//0x4e
	{"LD C, A",0,(void *)&ld_c_a},				//0x4f
	{"LD D, B",0,NULL},				//0x50
	{"LD D, C",0,NULL},				//0x51
	{"LD D, D",0,NULL},				//0x52
	{"LD D, E",0,NULL},				//0x53
	{"LD D, H",0,NULL},				//0x54
	{"LD D, L",0,NULL},				//0x55
	{"LD D, (HL)",0,NULL},			//0x56
	{"LD D, A",0,NULL},				//0x57
	{"LD E, B",0,NULL},				//0x58
	{"LD E, C",0,NULL},				//0x59
	{"LD E, D",0,NULL},				//0x5a
	{"LD E, E",0,NULL},				//0x5b
	{"LD E, H",0,NULL},				//0x5c
	{"LD E, L",0,NULL},				//0x5d
	{"LD E, (HL)",0,NULL},			//0x5e
	{"LD E, A",0,NULL},				//0x5f
	{"LD H, B",0,NULL},				//0x60
	{"LD H, C",0,NULL},				//0x61
	{"LD H, D",0,NULL},				//0x62
	{"LD H, E",0,NULL},				//0x63
	{"LD H, H",0,NULL},				//0x64
	{"LD H, L",0,NULL},				//0x65
	{"LD H, (HL)",0,NULL},			//0x66
	{"LD H, A",0,NULL},				//0x67
	{"LD L, B",0,NULL},				//0x68
	{"LD L, C",0,NULL},				//0x69
	{"LD L, D",0,NULL},				//0x6a
	{"LD L, E",0,NULL},				//0x6b
	{"LD L, H",0,NULL},				//0x6c
	{"LD L, L",0,NULL},				//0x6d
	{"LD L, (HL)",0,NULL},			//0x6e
	{"LD L, A",0,NULL},				//0x6f
	{"LD (HL), B",0,NULL},			//0x70
	{"LD (HL), C",0,NULL},			//0x71
	{"LD (HL), D",0,NULL},			//0x72
	{"LD (HL), E",0,NULL},			//0x73
	{"LD (HL), H",0,NULL},			//0x74
	{"LD (HL), L",0,NULL},			//0x75
	{"HALT",0,NULL},				//0x76
	{"LD (HL), A",0,NULL},			//0x77
	{"LD A, B",0,(void *)&ld_a_b},				//0x78
	{"LD A, C",0,(void *)&ld_a_c},				//0x79
	{"LD A, D",0,(void *)&ld_a_d},				//0x7a
	{"LD A, E",0,(void *)&ld_a_e},				//0x7b
	{"LD A, H",0,(void *)&ld_a_h},				//0x7c
	{"LD A, L",0,(void *)&ld_a_l},				//0x7d
	{"LD A, (HL)",0,(void *)&ld_a_hl},			//0x7e
	{"LD A, A",0,(void *)&nop},				//0x7f
	{"ADD A, B",0,(void *)&add_a_b},			//0x80
	{"ADD A, C",0,(void *)&add_a_c},			//0x81
	{"ADD A, D",0,(void *)&add_a_d},			//0x82
	{"ADD A, E",0,(void *)&add_a_e},			//0x83
	{"ADD A, H",0,(void *)&add_a_h},			//0x84
	{"ADD A, L",0,(void *)&add_a_l},			//0x85
	{"ADD A, (HL)",0,(void *)&add_a_hl},			//0x86
	{"ADD A, A",0,(void *)&add_a_a},				//0x87
	{"ADD B",0,NULL},				//0x88
	{"ADD C",0,NULL},				//0x89
	{"ADD D",0,NULL},				//0x8a
	{"ADD E",0,NULL},				//0x8b
	{"ADD H",0,NULL},				//0x8c
	{"ADD L",0,NULL},				//0x8d
	{"ADD (HL)",0,NULL},			//0x8e
	{"ADC A",0,NULL},				//0x8f
	{"SUB A, B",0,(void *)&sub_a_b},				//0x90
	{"SUB A, C",0,(void *)&sub_a_c},				//0x91
	{"SUB A, D",0,(void *)&sub_a_d},				//0x92
	{"SUB A, E",0,(void *)&sub_a_e},				//0x93
	{"SUB A, H",0,(void *)&sub_a_h},				//0x94
	{"SUB A, L",0,(void *)&sub_a_l},				//0x95
	{"SUB A, (HL)",0,(void *)&sub_a_hl},			//0x96
	{"SUB A, A",0,(void *)&sub_a_a},				//0x97
	{"SBC B",0,NULL},				//0x98
	{"SBC C",0,NULL},				//0x99
	{"SBC D",0,NULL},				//0x9a
	{"SBC E",0,NULL},				//0x9b
	{"SBC H",0,NULL},				//0x9c
	{"SBC L",0,NULL},				//0x9d
	{"SBC (HL)",0,NULL},			//0x9e
	{"SBC A",0,NULL},				//0x9f
	{"AND B",0,(void *)&and_b},				//0xa0
	{"AND C",0,(void *)&and_c},				//0xa1
	{"AND D",0,(void *)&and_d},				//0xa2
	{"AND E",0,(void *)&and_e},				//0xa3
	{"AND H",0,(void *)&and_h},				//0xa4
	{"AND L",0,(void *)&and_l},				//0xa5
	{"AND (HL)",0,(void *)&and_hl},			//0xa6
	{"AND A",0,(void *)&and_a},				//0xa7
	{"XOR B",0,(void *)&xor_b},				//0xa8
	{"XOR C",0,(void *)&xor_c},				//0xa9
	{"XOR D",0,(void *)&xor_d},				//0xaa
	{"XOR E",0,(void *)&xor_e},				//0xab
	{"XOR H",0,(void *)&xor_h},				//0xac
	{"XOR L",0,(void *)&xor_l},				//0xad
	{"XOR (HL)",0,(void *)&xor_hl},			//0xae
	{"XOR A",0,(void *)&xor_a},				//0xaf
	{"OR B",0,(void *)&or_b},				//0xb0
	{"OR C",0,(void *)&or_c},				//0xb1
	{"OR D",0,(void *)&or_d},				//0xb2
	{"OR E",0,(void *)&or_e},				//0xb3
	{"OR H",0,(void *)&or_h},				//0xb4
	{"OR L",0,(void *)&or_l},				//0xb5
	{"OR (HL)",0,(void *)&or_hl},				//0xb6
	{"OR A",0,(void *)&or_a},				//0xb7
	{"CP B",0,NULL},				//0xb8
	{"CP C",0,NULL},				//0xb9
	{"CP D",0,NULL},				//0xba
	{"CP E",0,NULL},				//0xbb
	{"CP H",0,NULL},				//0xbc
	{"CP L",0,NULL},				//0xbd
	{"CP (HL)",0,NULL},				//0xbe
	{"CP A",0,NULL},				//0xbf
	{"RET NZ",0,(void *)&ret_nz},				//0xc0
	{"POP BC",0,(void *)&pop_bc},				//0xc1
	{"JP NZ, 0x%04x",2,NULL},		//0xc2
	{"JP 0x%04x",2,(void *)&jp_nn},			//0xc3
	{"CALL NZ, 0x%04x",2,NULL},		//0xc4
	{"PUSH BC",0,(void *)&push_bc},				//0xc5
	{"ADD A, 0x%02x",1,NULL},		//0xc6
	{"RST 0x00",0,NULL},			//0xc7
	{"RET Z",0,(void *)&ret_z},				//0xc8
	{"RET",0,(void *)&ret},					//0xc9
	{"JP Z, 0x%04x",2,NULL},		//0xca
	{"CB 0x%02x",1,(void *)&cbEmulation},				//0xcb
	{"CALL Z, 0x%04x",2,NULL},		//0xcc
	{"CALL 0x%04x",2,(void *)&call_nn},			//0xcd
	{"ADC 0x%20x",1,NULL},			//0xce
	{"RST 0x08",0,(void *)&rst_8},			//0xcf
	{"RET NC",0,NULL},				//0xd0
	{"POP DE",0,(void *)&pop_de},				//0xd1
	{"JP NC, 0x%04x",2,NULL},		//0xd2
	{"UNKNOWN",0,NULL},				//0xd3
	{"CALL NC, 0x%04x",2,NULL},		//0xd4
	{"PUSH DE",0,(void *)&push_de},				//0xd5
	{"SUB 0x%20x",1,NULL},			//0xd6
	{"RST 0x10",0,NULL},			//0xd7
	{"RET C",0,NULL},				//0xd8
	{"RETI",0,(void *)&reti},				//0xd9
	{"JP C, 0x%04x",2,NULL},		//0xda
	{"UNKNOWN",0,NULL},				//0xdb
	{"CALL C, 0x%04x",2,NULL},		//0xdc
	{"UNKNOWN",0,NULL},				//0xdd
	{"SBC 0x%02x",1,NULL},			//0xde
	{"RST 0x18",0,(void *)&rst_18},			//0xdf
	{"LD (0xff00 + 0x%02x), A",1,(void *)&ldh_n_a},	//0xe0
	{"POP HL",0,(void *)&pop_hl},					//0xe1
	{"LD (0xff00 + C), A",0,(void *)&ldh_c_a},		//0xe2
	{"UNKNOWN",0,NULL},					//0xe3
	{"UNKNOWN",0,NULL},					//0xe4
	{"PUSH HL",0,(void *)&push_hl},					//0xe5
	{"AND 0x%02x",1,(void *)&and_n},				//0xe6
	{"RST 0x20",0,NULL},				//0xe7
	{"ADD SP, 0x%02x",1,NULL},			//0xe8
	{"JP HL",0,NULL},					//0xe9
	{"LD (0x%04x), A",2,(void *)&ld_nn_a},			//0xea
	{"UNKNOWN",0,NULL},					//0xeb
	{"UNKNOWN",0,NULL},					//0xec
	{"UNKNOWN",0,NULL},					//0xed
	{"XOR 0x%02x",1,NULL},				//0xee
	{"RST 0x28",0,(void *)&rst_28},				//0xef
	{"LD A, (0xff00 + 0x%02x)",1,(void *)&ldh_a_n},	//0xf0
	{"POP AF",0,(void *)&pop_af},					//0xf1
	{"LD A, (0xff00 + C)",0,NULL},		//0xf2
	{"DI",0,(void *)di},						//0xf3
	{"UNKNOWN",0,NULL},					//0xf4
	{"PUSH AF",0,(void *)&push_af},					//0xf5
	{"OR 0x%02x",1,NULL},				//0xf6
	{"RST 0x30",0,NULL},				//0xf7
	{"LD HL, SP+0x%02x",1,NULL},		//0xf8
	{"LD SP, HL",0,NULL},				//0xf9
	{"LD A, (0x%04x)",2,(void *)&ld_a_nn},			//0xfa
	{"EI",0,(void *)&ei},						//0xfb
	{"UNKNOWN",0,NULL},					//0xfc
	{"UNKNOWN",0,NULL},					//0xfd
	{"CP 0x%02x",1,(void *)&cp_n},				//0xfe
	{"RST 0x38",0,(void *)&rst_38},				//0xff
};

const unsigned char instructionTicks[256] = {
	2, 6, 4, 4, 2, 2, 4, 4, 10, 4, 4, 4, 2, 2, 4, 4, // 0x0_
	2, 6, 4, 4, 2, 2, 4, 4,  4, 4, 4, 4, 2, 2, 4, 4, // 0x1_
	0, 6, 4, 4, 2, 2, 4, 2,  0, 4, 4, 4, 2, 2, 4, 2, // 0x2_
	4, 6, 4, 4, 6, 6, 6, 2,  0, 4, 4, 4, 2, 2, 4, 2, // 0x3_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x4_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x5_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x6_
	4, 4, 4, 4, 4, 4, 2, 4,  2, 2, 2, 2, 2, 2, 4, 2, // 0x7_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x8_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0x9_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0xa_
	2, 2, 2, 2, 2, 2, 4, 2,  2, 2, 2, 2, 2, 2, 4, 2, // 0xb_
	0, 6, 0, 6, 0, 8, 4, 8,  0, 2, 0, 0, 0, 6, 4, 8, // 0xc_
	0, 6, 0, 0, 0, 8, 4, 8,  0, 8, 0, 0, 0, 0, 4, 8, // 0xd_
	6, 6, 4, 0, 0, 8, 4, 8,  8, 2, 8, 0, 0, 0, 4, 8, // 0xe_
	6, 6, 4, 2, 0, 8, 4, 8,  6, 4, 8, 2, 0, 0, 4, 8  // 0xf_
};


void reset(void){
	//setup each memory map
	memset(sram,0,sizeof(sram));
	memcpy(io,ioReset,sizeof(io));
	memset(vram,0,sizeof(vram));
	memset(oam,0,sizeof(oam));
	memset(wram,0,sizeof(wram));
	memset(hram,0,sizeof(hram));

	//setup general purpose registers
	regs.a = 0x01;
	regs.f = 0xb0;
	regs.b = 0x00;
	regs.c = 0x13;
	regs.d = 0x00;
	regs.e = 0xd8;
	regs.h = 0x01;
	regs.l = 0x4d;
	regs.sp = 0xfffe;
	regs.pc = 0x0100;

	//setup interrupt registers
	interrupt.master = 1;
	interrupt.enable = 0;
	interrupt.flags = 0;

	//setup keys
	keys.a = 1;
	keys.b = 1;
	keys.select = 1;
	keys.start = 1;
	keys.right = 1;
	keys.left = 1;
	keys.up = 1;
	keys.down = 1;
	
	memset(tiles,0,sizeof(tiles));

	backgroundPalette[0] = palette[0];
	backgroundPalette[1] = palette[1];
	backgroundPalette[2] = palette[2];
	backgroundPalette[3] = palette[3];

	spritePalette[0][0] = palette[0];
	spritePalette[0][1] = palette[1];
	spritePalette[0][2] = palette[2];
	spritePalette[0][3] = palette[3];

	spritePalette[1][0] = palette[0];
	spritePalette[1][1] = palette[1];
	spritePalette[1][2] = palette[2];
	spritePalette[1][3] = palette[3];

	ticks = 0;

	writeByte(0xff05,0x00);
	writeByte(0xff06,0x00);
	writeByte(0xff07,0x00);
	writeByte(0xff10,0x80);
	writeByte(0xff11,0xbf);
	writeByte(0xff12,0xf3);
	writeByte(0xff14,0xbf);
	writeByte(0xff16,0x3f);
	writeByte(0xff17,0x00);
	writeByte(0xff19,0xbf);
	writeByte(0xff1a,0x7a);
	writeByte(0xff1b,0xff);
	writeByte(0xff1c,0x9f);
	writeByte(0xff1e,0xbf);
	writeByte(0xff20,0xff);
	writeByte(0xff21,0x00);
	writeByte(0xff22,0x00);
	writeByte(0xff23,0xbf);
	writeByte(0xff24,0x77);
	writeByte(0xff25,0xf3);
	writeByte(0xff26,0xf1);
	writeByte(0xff40,0x91);
	writeByte(0xff42,0x00);
	writeByte(0xff43,0x00);
	writeByte(0xff45,0x00);
	writeByte(0xff47,0xfc);
	writeByte(0xff48,0xff);
	writeByte(0xff49,0xff);
	writeByte(0xff4a,0x00);
	writeByte(0xff4b,0x00);
	writeByte(0xffff,0x00);
}


void cpuEmulation(void){
	unsigned char instruction;
	unsigned short operand = 0;

	instruction = readByte(regs.pc++);

	switch(instructions[instruction].openrandlength){
		case 1:
			operand = (unsigned short)readByte(regs.pc);
			break;
		case 2:
			operand = readShort(regs.pc);
			break;
	}

	//for debug
	if(break_point)
		getchar();

	printf("regs.pc: 0x%2x, opcode: 0x%02x, ",regs.pc-1,instruction);
	switch(instructions[instruction].openrandlength){
		case 0:
			printf("operand: None");
			break;
		case 1:
			printf("operand: 0x%02x",operand);
			break;
		case 2:
			printf("operand: 0x%04x",operand);
			break;
	}
		
	printf("\t[");
	printf(instructions[instruction].disassembly,operand);
	printf("]\n");

	regs.pc += instructions[instruction].openrandlength;

	switch(instructions[instruction].openrandlength){
		case 0:
			((void (*)(void))instructions[instruction].handler)();
			break;	
		case 1:
			((void (*)(unsigned char))instructions[instruction].handler)((unsigned char)operand);
			break;
		case 2:
			((void (*)(unsigned short))instructions[instruction].handler)(operand);
			break;
	}

	ticks += instructionTicks[instruction];

}
