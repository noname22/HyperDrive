#ifndef DCPU16INS_H
#define DCPU16INS_H

typedef enum {
	// instructions with 1 operand
	DI_Jsr, DI_Sys,

	// instructions with 2 operands
	DI_Setl, DI_Setw, DI_Setb,
	DI_Add, DI_Sub, DI_Mul, DI_Div, DI_Mod, DI_Shl, 
	DI_Shr, DI_And, DI_Bor, DI_Xor, 
	
	DI_Ife, DI_Ifn, DI_Ifg, DI_Ifb,
} DIns;

// Remember to modify this when adding new instructions
// Should evaluate to true if the given instruction 
// modifies its target (ie. operand 0)

#define DINS_MODS_TARGET(_i) ((_i) >= DI_Setl && (_i) <= DI_Xor)

#define DINS_NUM (DI_Ifb + 1)
#define DINS_1OP_BASE DI_Jsr
#define DINS_2OP_BASE DI_Setl

#define DINSNAMES {"jsr", "sys", \
	"set.l", "set.w", "set.b", \
	"add", "sub", "mul", "div", "mod", "shl", "shr", \
	"and", "bor", "xor", "ife", "ifn", "ifg", "ifb"}

// Value encoding
typedef enum {
	DV_A, DV_B, DV_C, DV_X, DV_Y, DV_Z, DV_I, DV_J,
	DV_RefBase = 0x08, DV_RefTop = 0x0f,
	DV_RefRegNextWordBase = 0x10, DV_RefRegNextWordTop = 0x17,
	DV_Pop, DV_Peek, DV_Push,
	DV_SP, DV_PC,
	DV_O,
	DV_RefNextWord, DV_NextWord
} DVals;

#define DVALS_NUM (DV_NextWord + 1)

#define VALNAMES {\
	"a", "b", "c", "x", "y", "z", "i", "j",\
	"[a]", "[b]", "[c]", "[x]", "[y]", "[z]", "[i]", "[j]",\
	"[nw+a]", "[nw+b]", "[nw+c]", "[nw+x]", "[nw+y]", "[nw+z]", "[nw+i]", "[nw+j]",\
	"pop", "peek", "push",\
	"sp", "pc",\
	"o",\
	"[nw]", "nw"\
}

#endif
