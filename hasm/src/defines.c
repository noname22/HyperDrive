#include "hasmi.h"

typedef struct {
	char* search; 
	char* replace;
} Define;

typedef Vector(Define) DefineVec;

struct Defines {
	DefineVec defs;
};

Defines* Defines_Create()
{
	Defines* me = calloc(1, sizeof(Defines));
	LAssert(me, "could not allocate ram for defines");
	Vector_Init(me->defs, Define);
	
	return me;
}

void Defines_Push(Defines* me, const char* search, const char* replace)
{
	char tmp[strlen(replace)];

	Define def = {strdup(search), strdup(StrStrip(tmp, replace))};
	Vector_Add(me->defs, def);
}

void Defines_Pop(Defines* me, int num)
{
	Vector_Remove(me->defs, me->defs.count - 1 - num, num);
}

char* Defines_Replace(Defines* me, char* buffer, bool partials)
{
	if(partials){
		char* sub;
		Define* it;
		Vector_ForEach(me->defs, it){
			if((sub = strstr(buffer, it->search))){
				char tmp[MAX_STR_SIZE] = {0};
				StrReplace(tmp, sub, it->search, it->replace);
				strcpy(sub, tmp);
				return buffer;
			}
		}
	}else{
		Define* it;
		Vector_ForEach(me->defs, it){
			if(!strcmp(it->search, buffer)){
				strcpy(buffer, it->replace);
				return buffer;
			}
		}
	}

	return buffer;
}
