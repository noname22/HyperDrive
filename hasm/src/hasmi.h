#ifndef HASMI_H
#define HASMI_H

#define LDEBUG

#include "common.h"
#include "hasm.h"

#define MAX_STR_SIZE 8192
#define LAssertError(__v, ...) \
	if(!(__v)){ \
		if(me->reader) LogI("@ %s:%d", Reader_GetFilename(me->reader), Reader_GetLineNumber(me->reader)); \
		LogF(__VA_ARGS__); \
		exit(1);\
	}

typedef Vector(char) CharVec;

typedef char* CharPtr;
typedef Vector(CharPtr) StrVec;

struct Macro
{
	char* name;
	char* filename;
	int sourceLine; // Originating line
	CharVec str;	// The macro
	StrVec args;
};

Macro* Macro_Create(const char* name, const char* filename, int sourceLine);
void Macro_AddArg(Macro* me, const char* arg);
bool Macro_AddLine(Macro* me, Hasm* hasm, const char* line);
Reader* Macro_GetReader(Macro* me);

typedef struct MacroCall MacroCall;
MacroCall* MacroCall_Create(Macro* macro);
void MacroCall_PushArgs(MacroCall* me, Defines* defs);
void MacroCall_PopArgs(MacroCall* me, Defines* defs);
void MacroCall_AddCallArg(MacroCall* me, const char* arg); 
void MacroCall_Destroy(MacroCall** me);

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

	const char* filename;
	int lineNumber;

	LabelRefs references;
} Label;
Vector(Label);

Labels* Labels_Create();
Label* Labels_Lookup(Labels* me, const char* label);
Label* Labels_Add(Labels* me, const char* label);
void Labels_Define(Labels* me, Hasm* d, const char* label, uint32_t address, const char* filename, int lineNumber);
uint32_t Labels_Get(Labels* me, const char* label, uint32_t current, uint32_t insAddr, const char* filename, int lineNumber);
void Labels_Replace(Labels* me, uint8_t* ram);

int Defines_GetCount(Defines* me);
void Defines_Print(Defines* me);
const char* Defines_Get(Defines* me, const char* name);
void Defines_Set(Defines* me, const char* search, const char* replace);

Reader* Reader_CreateFromFile(const char* filename);
Reader* Reader_CreateFromBuffer(char* buffer, int len, const char* name, int startLineNum);
int Reader_GetLineNumber(Reader* me);
bool Reader_GetLine(Reader* me, char* buffer);
const char* Reader_GetFilename(Reader* me);
void Reader_Destroy(Reader** me);

bool GetLine(Hasm* me, FILE* f, char* buffer);
char* GetToken(Hasm* me, char* buffer, char* token);
uint32_t Assemble(Hasm* me, Reader* reader, int addr, int depth);

Defines* Defines_Create();
bool Defines_Push(Defines* me, const char* search, const char* replace);
void Defines_Pop(Defines* me, int num);
char* Defines_Replace(Defines* me, char* buffer);

#endif
