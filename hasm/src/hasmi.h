#ifndef HASMI_H
#define HASMI_H

#include "common.h"
#include "hasm.h"

#define MAX_STR_SIZE 8192
#define LAssertError(__v, ...) \
	if(!(__v)){ \
		if(me->currentFile) LogI("@ %s:%d", me->currentFile, me->lineNumber); \
		LogF(__VA_ARGS__); \
		exit(1);\
	}

typedef Vector(char) CharVec;

typedef struct 
{
	int sourceLine; // Originating line
	CharVec str;	// The macro
} Macro;

typedef Macro* MacroPtr;
typedef Vector(MacroPtr) MacroVec;

typedef struct { char* searchReplace[2]; } Define;
Vector(Define);

typedef struct
{
	uint32_t addr;
	uint32_t insAddr;
	char* filename;
	int lineNumber;
	bool relative;
} LabelRef;

typedef Vector(LabelRef) LabelRefs;

typedef struct {
	char* label;

	uint32_t addr;
	uint32_t id;
	bool found;

	int lineNumber;
	char* filename;

	LabelRefs references;
} Label;

Vector(Label); 

Labels* Labels_Create();
Label* Labels_Lookup(Labels* me, const char* label);
Label* Labels_Add(Labels* me, const char* label);
void Labels_Define(Labels* me, Hasm* d, const char* label, uint32_t address, const char* filename, int lineNumber);
uint32_t Labels_Get(Labels* me, const char* label, uint32_t current, uint32_t insAddr, const char* filename, int lineNumber);
void Labels_Replace(Labels* me, uint8_t* ram);
bool GetLine(Hasm* me, FILE* f, char* buffer);
char* GetToken(Hasm* me, char* buffer, char* token);
uint32_t Assemble(Hasm* me, const char* ifilename, int addr, int depth);

#endif
