#include "hasmi.h"

Macro* Macro_Create(const char* name, const char* filename, int sourceLine)
{
	Macro* me = calloc(1, sizeof(Macro));
	LAssert(me, "could not allocate ram for macro");

	Vector_Init(me->str, char);
	Vector_Init(me->args, CharPtr);

	me->name = strdup(name);
	me->filename = strdup(filename);
	me->sourceLine = sourceLine;

	return me;
}

bool Macro_AddLine(Macro* me, Hasm* hasm, const char* line)
{
	char token[MAX_STR_SIZE] = {0};
	char* l = strdup(line);
	GetToken(hasm, l, token);
	free(l);

	if(!strcmp(token, ".end"))
		return true;

	int len = strlen(line);
	for(int i = 0; i < len; i++){
		Vector_Add(me->str, line[i]);
	}
	Vector_Add(me->str, '\n');;

	LogD("macro now conatins:\n%s", me->str.elems);

	return false;
}

void Macro_AddArg(Macro* me, const char* arg)
{
	Vector_Add(me->args, strdup(arg));
}

Reader* Macro_GetReader(Macro* me)
{
	return Reader_CreateFromBuffer(me->str.elems, me->str.count, me->filename, me->sourceLine);
}

struct MacroCall {
	StrVec args;
	Macro* macro;
};

MacroCall* MacroCall_Create(Macro* macro)
{
	MacroCall* me = calloc(1, sizeof(MacroCall));
	LAssert(me, "could not allocate ram for macrocall");

	me->macro = macro;
	Vector_Init(me->args, CharPtr);

	return me;
}

void MacroCall_PushArgs(MacroCall* me, Defines* defs)
{
	LAssert(me->macro->args.count == me->args.count, "macro %s expects %d arguments, %d given", 
		me->macro->name, me->macro->args.count, me->args.count);

	LogD("before push args: %d", Defines_GetCount(defs));
	Defines_Print(defs);

	for(int i = 0; i < me->args.count; i++)
		Defines_Push(defs, me->macro->args.elems[i], me->args.elems[i]);
	
	LogD("after push args: %d", Defines_GetCount(defs));
	Defines_Print(defs);
}

void MacroCall_PopArgs(MacroCall* me, Defines* defs)
{
	Defines_Pop(defs, me->args.count);
	LogD("after pop args: %d", Defines_GetCount(defs));
	Defines_Print(defs);
}

void MacroCall_AddCallArg(MacroCall* me, const char* arg)
{
	Vector_Add(me->args, strdup(arg));
}

void MacroCall_Destroy(MacroCall** me)
{
	CharPtr* it;
	Vector_ForEach((*me)->args, it){
		free(*it);
	}
	Vector_Free((*me)->args);

	free(*me);
	*me = NULL;
}
