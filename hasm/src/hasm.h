#ifndef DASM_H
#define DASM_H

#define ENDSWITH(__str, __c) (__str)[strlen(__str) - 1] == (__c)
#define STARTSWITH(__str, __c) ((__str)[0] == (__c))

#include <stdint.h>
#include <stdio.h>

extern int logLevel;

typedef struct Define_vec_s Defines;
typedef struct Label_vec_s Labels;

typedef struct {
	const char* currentFile;
	char* baseDir;
	int lineNumber;

	uint8_t* ram;

	uint32_t endAddr;

	Defines* defines;
	Labels* labels;

	FILE* debugFile;
} Hasm;

Hasm* Hasm_Create();
void Hasm_Destroy(Hasm** hasm);
uint32_t Hasm_Assemble(Hasm* me, const char* ifilename, uint8_t* ram, int startAddr, uint32_t endAddr);

#endif
