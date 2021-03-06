#include "hasmi.h"

int GetDir(const char* filename, char* buffer)
{
	int last = 0;
	for(int i = 0; i < strlen(filename); i++){
		if(filename[i] == '\\' || filename[i] == '/') last = i + 1;
	}
	
	if(buffer) strncpy(buffer, filename, last);

	return last;
}

Hasm* Hasm_Create()
{
	Hasm* me = calloc(1, sizeof(Hasm));
	LAssert(me, "Could not allocate RAM for assembler");
	
	me->labels = Labels_Create();
	me->defines = Defines_Create();

	Vector_Init(me->macros, MacroPtr);

	return me;
}

void Hasm_Destroy(Hasm** me)
{
	free(*me);
	*me = NULL;
}

uint32_t Hasm_Assemble(Hasm* me, const char* ifilename, uint8_t* ram, int startAddr, uint32_t endAddr)
{
	me->ram = ram;
	me->endAddr = endAddr;

	if(me->baseDir) free(me->baseDir);
	me->baseDir = calloc(1, GetDir(ifilename, NULL));
	GetDir(ifilename, me->baseDir);

	Reader* r = Reader_CreateFromFile(ifilename);
	LAssert(r, "could not open file: %s", ifilename);
	uint32_t ret = Assemble(me, r, startAddr, 0);
	Reader_Destroy(&r);

	Labels_Replace(me->labels, me->ram);

	return ret;
}
