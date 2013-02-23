#ifndef DCPU16INS_H
#define DCPU16INS_H

typedef enum {
	// instructions with 1 operand
	DI_Jsr, DI_Sys,

	// instructions with 2 operands
	DI_Set, DI_Add, DI_Sub, DI_Mul, DI_Div, DI_Mod, DI_Shl, 
	DI_Shr, DI_And, DI_Bor, DI_Xor, DI_Ife, DI_Ifn, DI_Ifg, DI_Ifb,
} DIns;

#define DINS_NUM (DI_Ifb + 1)
#define DINS_1OP_BASE DI_Jsr
#define DINS_2OP_BASE DI_Set

#define DINSNAMES {"JSR", "SYS", "SET", "ADD", "SUB", "MUL", "DIV", "MOD", "SHL", "SHR", \
	"AND", "BOR", "XOR", "IFE", "IFN", "IFG", "IFB"}

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
	"A", "B", "C", "X", "Y", "Z", "I", "J",\
	"[A]", "[B]", "[C]", "[X]", "[Y]", "[Z]", "[I]", "[J]",\
	"[NW+A]", "[NW+B]", "[NW+C]", "[NW+X]", "[NW+Y]", "[NW+Z]", "[NW+I]", "[NW+J]",\
	"POP", "PEEK", "PUSH",\
	"SP", "PC",\
	"O",\
	"[NW]", "NW"\
}

#endif
