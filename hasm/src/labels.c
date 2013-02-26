#include "hasmi.h"

#define REL "rel:"

bool IsRelative(const char* s)
{
	if(strlen(s) < strlen(REL)) return false;
	return !strncmp(s, REL, strlen(REL));
}

const char* GetName(const char* s)
{
	if(IsRelative(s)) return s + strlen(REL);
	return s;
}

Labels* Labels_Create()
{
	Labels* me = calloc(1, sizeof(Labels));
	Vector_Init(*me, Label);
	return me;
}

// XXX: use a hash map or something
Label* Labels_Lookup(Labels* me, const char* label)
{
	Label* it;
	Vector_ForEach(*me, it){
		if(!strcmp(it->label, GetName(label))) return it;
	}

	return NULL;
}

Label* Labels_Add(Labels* me, const char* label)
{
	label = GetName(label);
	
	// XXX: Why do I have two duplicate lable checks?
	LAssert(Labels_Lookup(me, label) == NULL, "duplicate label/constant: %s", label);

	Label l;
	memset(&l, 0, sizeof(Label));
	l.label = strdup(label);
	l.id = me->count;
	Vector_Init(l.references, LabelRef);

	Vector_Add(*me, l);

	return &me->elems[me->count - 1];
}

void Labels_Define(Labels* lme, Hasm* me, const char* label, uint32_t address, const char* filename, int lineNumber)
{
	label = GetName(label);
	LogD("defining label: %s", label);

	Label* l = Labels_Lookup(lme, label);
	if(!l) l = Labels_Add(lme, label);
	else 
		LAssertError(!l->found, 
			"duplicate label/constant: %s, first defined at %s:%d", 
			label, l->filename, l->lineNumber);

	l->addr = address;
	l->found = true;
	l->filename = strdup(filename);
	l->lineNumber = lineNumber;
} 

uint32_t Labels_Get(Labels* me, const char* label, uint32_t current, uint32_t insAddr, const char* filename, int lineNumber)
{
	bool isRelative = IsRelative(label);
	label = GetName(label);

	Label* l = Labels_Lookup(me, label);
	if(!l) l = Labels_Add(me, label);

	LabelRef ref;

	ref.lineNumber = lineNumber;
	ref.filename = strdup(filename);
	ref.addr = current;
	ref.insAddr = insAddr;
	ref.relative = isRelative;

	Vector_Add(l->references, ref);

	return l->id;
}
	
void Labels_Replace(Labels* me, uint8_t* ram)
{
	LogD("replacing labels");

	LogD("label count: %d", me->count);

	Label* l;
	Vector_ForEach(*me, l){
		if(!l->found){
			LogF("No such label/constant: %s", l->label);
			LogI("Referenced from:");

			LabelRef* ref;
			Vector_ForEach(l->references, ref){
				LogI("  %s:%d", ref->filename, ref->lineNumber);
			}
			exit(1);
		}
		LogD("label: %s", l->label);

		LabelRef* ref;
		Vector_ForEach(l->references, ref){
			const char* relname[] = {"absolute", "relative"};
			uint32_t addr = ref->relative ? -(ref->addr - l->addr) - 1: l->addr;
			uint32_t at = ref->addr;

			ram[at++] = addr & 0xff;
			ram[at++] = (addr >> 8) & 0xff;
			ram[at++] = (addr >> 16) & 0xff;
			ram[at]   = (addr >> 24) & 0xff;

			LogD("replaced label %s @ 0x%04x with %s address 0x%04x", 
				l->label, ref->addr, relname[ref->relative], addr);
		}
	}
}
