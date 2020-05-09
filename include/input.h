#ifndef _INC_INPUT
#define _INC_INPUT

/*
struct keys1 {
	unsigned char start;
	unsigned char select;
	unsigned char b;
	unsigned char a;
};

struct keys2 {
	unsigned char down;
	unsigned char up;
	unsigned char left;
	unsigned char right;
};

struct keys {
	union {
		struct {
			union {
				struct keys1 key1;
				unsigned char keys1;
			};
			
			union {
				struct keys2 key2;
				unsigned char keys2;
			};
		};
		
		unsigned char c;
	};
};
*/

struct keys {
	unsigned char start;
	unsigned char select;
	unsigned char b;
	unsigned char a;
	unsigned char down;
	unsigned char up;
	unsigned char left;
	unsigned char right;
};

extern struct keys keys;

#endif
