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

bool Defines_Push(Defines* me, const char* search, const char* replace)
{
	char tmp[strlen(replace)];
	Define def = {strdup(search), strdup(StrStrip(tmp, replace))};
	Vector_Add(me->defs, def);

	return true;
}

void Defines_Pop(Defines* me, int num)
{
	LogD("popping %d args at %d (of %d)", num, me->defs.count - num, me->defs.count);
	Vector_Remove(me->defs, me->defs.count - num, num);
}

void Defines_Set(Defines* me, const char* search, const char* replace)
{
	Define* it;
	Vector_ReverseForEach(me->defs, it){
		if(!strcmp(it->search, search)){
			free(it->replace);
			it->replace = strdup(replace);
			return;
		}
	}

	Defines_Push(me, search, replace);
}

int Defines_GetCount(Defines* me)
{
	return me->defs.count;
}

const char* Defines_Get(Defines* me, const char* name)
{
	Define* it;
	Vector_ReverseForEach(me->defs, it){
		if(!strcmp(it->search, name)){
			return it->replace;
		}
	}

	return NULL;
}

void Defines_Print(Defines* me)
{
	Define* it;
	LogD("defines: ");
	Vector_ReverseForEach(me->defs, it){
		LogD("  %s -> %s", it->search, it->replace);
	}
}

static char* GetSubToken(char* buffer, int* len, int* skipped){
	*len = 0;
	*skipped = 0;

	if(*buffer == 0) return buffer;

	while(!(isalnum(*buffer) || *buffer == '_' || *buffer == '#') && *buffer != 0){
		buffer++;
		(*skipped)++;
	}

	if(*buffer == '#'){
		*len = 1;
		return buffer;
	}

	if(*buffer == 0) return buffer;

	char* b = buffer;

	while(isalnum(*b) || *b == '_' || *b == '.'){
		b++;
		(*len)++;
	}

	return buffer;
}

char* Defines_Replace(Defines* me, char* buffer)
{
	// strings, don't touch
	if(STARTSWITH(buffer, '"') || STARTSWITH(buffer, '\''))
		return buffer;
	
	LogD("before Define_Replace: %s", buffer);
	//Defines_Print(me);		

	char tmp[MAX_STR_SIZE] = {0};
	strcpy(tmp, buffer);

	char* w = buffer;

	char* token = tmp;

	#define WRITE(_v, _len) { for(int ii = 0; ii < _len; ii++){ \
		LAssert(w < buffer + MAX_STR_SIZE, "Internal assembler error in define replacer. Sorry :(");\
		 *(w++) = (_v)[ii]; }}

	int len = 0, skipped = 0;
	for(;;){
		token = GetSubToken(token, &len, &skipped);

		//LogD("%.*s", len, token);

		// write the skipped chars
		if(skipped)
			WRITE(token - skipped, skipped);

		if(len == 0)
			break;

		// "glue" the expresisons hello#goodbye becomes hellogoodbye
		if(*token == '#'){
			token++;
			continue;
		}

		Define* it;
		bool found = false;
		Vector_ReverseForEach(me->defs, it){
			// token matches the define
			if(strlen(it->search) == len && !strncmp(it->search, token, len)){
				// write the define instead of the token
				//LogD("replace: %s", it->replace);
				WRITE(it->replace, strlen(it->replace));
				found = true;
				break;
			}
		}

		if(!found){
			// write back the token
			//LogD("token: %.*s", len, token);
			WRITE(token, len);
		}

		token += len;
	}

	*w = 0;
	LogD("after Define_Replace: %s", buffer);
	return buffer;
}
