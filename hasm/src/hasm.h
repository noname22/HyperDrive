#ifndef DASM_H
#define DASM_H

#define ENDSWITH(__str, __c) (__str)[strlen(__str) - 1] == (__c)
#define STARTSWITH(__str, __c) ((__str)[0] == (__c))

#include <stdint.h>
#include <stdio.h>

extern int logLevel;

typedef struct Label_vec_s Labels;
typedef struct Macro Macro;
typedef Macro* MacroPtr;
typedef Vector(MacroPtr) MacroVec;
typedef struct Reader Reader;

typedef struct {
	char* baseDir;

	uint8_t* ram;

	uint32_t endAddr;
	Reader* reader;

	Labels* labels;
	MacroVec macros;

	FILE* debugFile;
} Hasm;

Hasm* Hasm_Create();
void Hasm_Destroy(Hasm** hasm);
uint32_t Hasm_Assemble(Hasm* me, const char* ifilename, uint8_t* ram, int startAddr, uint32_t endAddr);
int Tests(int argc, char** argv);

#endif
