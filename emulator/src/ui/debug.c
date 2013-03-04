#define LDEBUG

#include "cpu.h"
#include "debug.h"
#include "common.h"
#include "mem.h"

#include <stdlib.h>
	
#define RAssert(__v, ...) if(!(__v)){ DPRINT(__VA_ARGS__); return; }

typedef char* CharPtr;

typedef Vector(CharPtr) CharPtrVec;
typedef Vector(char) CharVec;
typedef Vector(int) IntVec;

typedef struct {
	uint32_t addr;
	bool enabled;
} BreakPoint;

typedef Vector(BreakPoint) BreakPointVec;

typedef struct {
	CharVec file;
	IntVec lineIndices;
	char* filename;
} SourceFile;

typedef SourceFile* SourceFilePtr;

typedef Vector(SourceFilePtr) SourceFilePtrVec;

typedef struct {
	uint32_t addr, length;
	int line;
	SourceFile* sourceFile;

	CharPtrVec items;
} DebugSymbol;

typedef Vector(DebugSymbol) DebugSymbolVec;
	
typedef struct {
	const char* cmd;
	void (*fun)(Debug* me, int argc, char** argv);
	const char* desc;
	const char* help;
} Command;

struct Debug {
	int runInstructions;
	int insExec;
	Cpu* cpu;
	Mem* mem;

	SourceFilePtrVec sourceFiles;
	DebugSymbolVec debugSymbols;
	BreakPointVec breakPoints;
	Command* commands;
	int nCommands;

	void (*Print)(Debug* me, void* data, const char* format, ...);
	void* printData;

	bool showNextIns;
};

#define DPRINT(...) me->Print(me, me->printData, __VA_ARGS__)

static const char unknown[] = "(\?\?\?\?)";

void Debug_DefaultPrintHandler(Debug* me, void* data, const char* format, ...){
	LogW("no print handler set for debugger");
}

void Debug_SetPrintfHandler(Debug* me, void (*fun)(Debug* me, void* data, const char* format, ...), void* data)
{
	me->Print = fun;
	me->printData = data;
}

SourceFile* Debug_GetSourceFileByName(Debug* me, const char* filename)
{
	SourceFile** sit;
	Vector_ForEach(me->sourceFiles, sit){
		if(!strcmp((*sit)->filename, filename)){
			return *sit;
		}
	}

	return NULL;
}

DebugSymbol* Debug_GetDebugSymbolByItem(Debug* me, const char* item)
{
	DebugSymbol* it;
	Vector_ForEach(me->debugSymbols, it){
		char** iit;
		Vector_ForEach(it->items, iit){
			if(!strcmp(item, *iit)) return it;
		}
	}

	return NULL;
}

bool Debug_GetRunning(Debug* me)
{
	return me->runInstructions == -1;
}

const char* SourceFile_GetLine(SourceFile* me, int line)
{
	if(line - 1 >= me->lineIndices.count) return unknown;
	return me->file.elems + me->lineIndices.elems[line - 1];
}

void Debug_AddBreakPointAddr(Debug* me, uint32_t addr)
{
	BreakPoint bp = {addr, true};
	Vector_Add(me->breakPoints, bp);
}

bool Debug_AddBreakPointLine(Debug* me, const char* filename, int line)
{
	SourceFile* sf = Debug_GetSourceFileByName(me, filename);
	if(!sf) return false;

	DebugSymbol* it;
	int lastLine = 0;
	Vector_ForEach(me->debugSymbols, it){
		if(sf == it->sourceFile && line > lastLine && line <= it->line){
			Debug_AddBreakPointAddr(me, it->addr);
			return true;
		}
		lastLine = it->line; 
	}

	return false;
}

bool Debug_AddBreakPointItem(Debug* me, const char* item)
{
	DebugSymbol* it;
	Vector_ForEach(me->debugSymbols, it){
		char** iit;
		Vector_ForEach(it->items, iit){
			if(!strcmp(item, *iit)){
				Debug_AddBreakPointAddr(me, it->addr);
				return true;
			}
		}
	}
	
	return false;
}

bool Debug_RemoveBreakPoint(Debug* me, int index)
{
	if(index < me->breakPoints.count){
		Vector_Remove(me->breakPoints, index, 1);
		return true;
	}
	return false;
}

void Debug_PrintBreakPoint(Debug* me, BreakPoint* bp)
{
	const char* onoff[] = {"disabled", "enabled"};
	DPRINT("0x%08x %s", bp->addr, onoff[bp->enabled]);
}

bool Debug_EnableBreakPoint(Debug* me, int index, bool enabled)
{
	if(index < me->breakPoints.count){
		me->breakPoints.elems[index].enabled = enabled;
		return true;
	}
	return false;
}

DebugSymbol* GetSymbolFromAddress(Debug* me, uint32_t addr)
{
	DebugSymbol* it;
	Vector_ForEach(me->debugSymbols, it){
		if(addr >= it->addr && addr < it->addr + it->length){
			return it;
		}
	}

	return NULL;
}

void Debug_Break(Debug* me)
{
	me->runInstructions = 0;
}

void Debug_Continue(Debug* me)
{
	me->runInstructions = -1;
}
	
void Continue(Debug* me, int argc, char** argv){
	if(argc == 1){
		me->runInstructions = -1;
		return;
	}
	
	unsigned int numIns = 0;
	RAssert(sscanf(argv[1], "%u", &numIns) == 1, "expected literal\n");

	me->runInstructions = numIns;
}
	
void Run(Debug* me, int argc, char** argv)
{
	Cpu_SetExit(me->cpu, false);
	Cpu_SetRegister(me->cpu, DR_PC, 0);
	Continue(me, argc, argv);
}
	
void Where(Debug* me, int argc, char** argv)
{
	uint32_t addr = Cpu_GetRegister(me->cpu, DR_PC);
	DebugSymbol* s = GetSymbolFromAddress(me, addr);
	if(!s){
		DPRINT("0x%04x: unknown\n", addr);
		return;
	}
	
	DPRINT("%08x (%08x-%08x) %s:%d %s\n", 
		addr, s->addr, s->addr + s->length,s->sourceFile->filename, s->line, 
		SourceFile_GetLine(s->sourceFile, s->line));
}

void Step(Debug* me, int argc, char** argv){
	me->runInstructions = 1;
	me->showNextIns = true;
}

void Break(Debug* me, int argc, char** argv){
	RAssert(argc >= 2, "break expects at least 1 argument, see help break\n");
	RAssert((!strcmp(argv[1], "list") && argc == 2) || argc == 3, 
		"invalid action or number of arguments, see help break\n");

	#define AddBreakPointAddr(_addr) Debug_AddBreakPointAddr(me, _addr); DPRINT("added breakpoint at address 0x%08x\n", _addr);

	#define AddBreakPointLine(_filename, _line) \
		if(!Debug_AddBreakPointLine(me, _filename, _line))\
			DPRINT("could not locate line %d of file '%s' in debug symbols\n", _line, _filename); \
		DPRINT("added breakpoint at %s:%d\n", _filename, _line); \

	if(!strcmp(argv[1], "add")){
		unsigned addr;
		char buffer[1024];

		if(sscanf(argv[2], "+0x%x", &addr) ||  sscanf(argv[2], "+%u", &addr) == 1){
			AddBreakPointAddr(Cpu_GetRegister(me->cpu, DR_PC) + addr);
		}

		else if(sscanf(argv[2], "-0x%x", &addr) ||  sscanf(argv[2], "-%u", &addr) == 1){
			AddBreakPointAddr(Cpu_GetRegister(me->cpu, DR_PC) - addr);
		}

		else if(sscanf(argv[2], "*0x%x", &addr) ||  sscanf(argv[2], "*%u", &addr) == 1){
			AddBreakPointAddr(addr);
		}

		else if(sscanf(argv[2], "%[^:]:%u", buffer, &addr) == 2){
			AddBreakPointLine(buffer, addr);
		}

		else if(sscanf(argv[2], "%u", &addr) == 1){
			DebugSymbol* s = GetSymbolFromAddress(me, Cpu_GetRegister(me->cpu, DR_PC));
			RAssert(s, "when trying to associate line number %d with a source file: " 
				"current address (pc) not associated with a source file, please specify source file\n"
				"(or use 'break add *%d' if you mean an address)\n", addr, addr);
			AddBreakPointLine(s->sourceFile->filename, addr);
		}
	
		else{
			RAssert(Debug_AddBreakPointItem(me, argv[2]), 
				"could not locate function/label '%s' in any of the source files\n", argv[2]);
			DPRINT("added breakpoint at function/label '%s'\n", argv[2]);
		}
	}

	else if(!strcmp(argv[1], "remove")){
		RAssert( Debug_RemoveBreakPoint(me, atoi(argv[2])), "invalid index\n");
		DPRINT("removed breakpoint: %d\n", atoi(argv[2]));
	}

	else if(!strcmp(argv[1], "enable")){
		RAssert( Debug_EnableBreakPoint(me, atoi(argv[2]), true), "invalid index\n");
		DPRINT("enabled breakpoint: %d\n", atoi(argv[2]));
	}

	else if(!strcmp(argv[1], "disable")){
		RAssert( Debug_EnableBreakPoint(me, atoi(argv[2]), false), "invalid index\n");
		DPRINT("disabled breakpoint: %d\n", atoi(argv[2]));
	}

	else if(!strcmp(argv[1], "list")){
		DPRINT("  current breakpoints:\n");
		BreakPoint* it;
		int i = 0;
		Vector_ForEach(me->breakPoints, it){
			DPRINT("    %2d. ", i++);
			Debug_PrintBreakPoint(me, it);
			DPRINT("\n");
		}
	}

	else {
		DPRINT("no such breakpoint action: '%s', see help", argv[1]);
	}
}

void Quit(Debug* me, int argc, char** argv){
	exit(0); 
}

typedef struct {const char* what; int v; } Printable;
int Lookup(Printable* printables, const char* str){
	if(str[0] == '@') return -4;
	for(int i = 0; i < sizeof(printables) / sizeof(Printable); i++){
		if(!strcmp(printables[i].what, str)) return printables[i].v;
	}
	return -1;
}

void Print(Debug* me, int argc, char** argv){
	RAssert(argc >= 2, "print requires at least 1 arguments\n");

	Printable printables[] = {
		{"a", DR_A}, {"b", DR_B}, {"c", DR_C}, {"x", DR_X}, {"y", DR_Y}, {"z", DR_Z}, 
		{"i", DR_I}, {"j", DR_J}, {"sp", DR_SP}, {"pc", DR_PC}, {"o", DR_O}, 
		{"regs", -2}, {"stack", -3}, {"@address", -4}
	};

	for(int i = 1; i < argc; i++){
		int what = Lookup(printables, argv[i]);
	
		if(what == -1){
			unsigned addr = 0;
			if(sscanf(argv[i], "0x%x", &addr) == 1 || sscanf(argv[i], "%u", &addr)){
				DPRINT("[0x%08x]: 0x%02x\n", addr, MEM_READ8(me->mem, addr));
			}

			else{
				DebugSymbol* s = Debug_GetDebugSymbolByItem(me, argv[i]);

				if(s){
					DPRINT("[%s (0x%08x)]: 0x%02x\n", argv[i], s->addr, MEM_READ8(me->mem, s->addr));
				}
				else DPRINT("I don't know what a '%s' is\n", argv[i]);
			}
		}

		if(what >= 0) DPRINT("%s: 0x%08x\n", argv[i], Cpu_GetRegister(me->cpu, what));

		// regs
		else if(what == -2){
			for(int i = 0; i <= DR_O; i++) DPRINT("%s: 0x%08x ", printables[i].what, Cpu_GetRegister(me->cpu, i));
			DPRINT("\n");
		}

		// stack
		else if(what == -3){
			uint32_t sp = Cpu_GetRegister(me->cpu, DR_SP);

			if(sp){
				for(int i = MEM_BOS; i >= sp; i--){
					DPRINT("  0x%08x\n", MEM_READ32(me->mem, i));
				}
			}else DPRINT("  (empty)\n");
		}

		else if(what == -4){
			unsigned addr;
			if(sscanf(argv[i], "@0x%x", &addr) == 1 || sscanf(argv[i], "@%u", &addr) == 1){
				DPRINT("[%08x] 8/0x%02x 16/0x%04x 32/0x%08x\n", 
					addr, MEM_READ8(me->mem, addr), MEM_READ16(me->mem, addr), MEM_READ32(me->mem, addr));
			}else{
				DPRINT("could not parse address %s", argv[i]);
			}
		}
	}
}

void Help(Debug* me, int argc, char** argv)
{
	if(argc == 1){
		char n = 0;
		DPRINT("\navailable me->commands:\n");
		for(int i = 0; i < me->nCommands; i++){
			DPRINT("  %s %*s %s\n", me->commands[i].cmd, 
				(int)(10 - strlen(me->commands[i].cmd)), &n, me->commands[i].desc);
		}
		DPRINT("\nNon-ambiguous short-hands are also accepted, such as s for step.\n");
	}else{
		for(int i = 0; i < me->nCommands; i++){
			if(!strcmp(me->commands[i].cmd, argv[1])){
				DPRINT("\n%s usage:\n%s\n", me->commands[i].cmd, me->commands[i].help);
				return;
			}
		}
		DPRINT("no help for: %s\n", argv[1]);
	}
}

void Debug_HandleInput(Debug* me, const char* text)
{
	Cpu* cpu = me->cpu;
	char input[512];
	strncpy(input, text, sizeof(input));
	
	BreakPoint* bit;
	Vector_ForEach(me->breakPoints, bit){
		if(Cpu_GetRegister(cpu, DR_PC) == bit->addr && bit->enabled){
			DPRINT("hit breakpoint: ");
			Debug_PrintBreakPoint(me, bit);
			DPRINT("\n");
			me->runInstructions = 0;
		}
	}

	// XXX get rid of the nested functions. It's a GCC extension, not C99.


	Command commands[] = {
		{"help",  NULL, "this command",
			"  help            lists all commands\n"
			"  help [command]  shows help about [command]\n"
		},

		{"break", Break, "adds, enables, disables or removes a breakpoint",
			"  adding breakpoints\n"
			"    break add *[addr]          adds a breakpoint at address [addr]\n"
			"    break add +[offset]        adds a breakpoint at current pc + [offset]\n"
			"    break add -[offset]        adds a breakpoint at current pc - [offset]\n"
			"    break add [source]:[line]  adds a breakpoint at the given line [line] in the source file [source]\n"
			"    break add [function]       adds a breakpoint at the given [function] (label)\n"
			"\n"
			"  managing breakpoints\n"
			"    break list                 lists and enumerates breakpoints\n"
			"    break remove [index]       removes breakpoint at given index\n"
			"    break enable [index]       enables breakpoint at given index\n"
			"    break disable [index]      disables breakpoint at given index\n"
		},

		{"continue", Continue, "continues execution",
			"  continue ([n ins])   continues execution and runs the specified number of instructions,"
			" or forever if nothing is specified.\n"},

		{"print", Print, "prints status information",
			"  print [r] ([r], [r]...) where r is a|b|c|x|y|z|i|j|o|sp|pc|regs|stack|@[addr]|@[0xhexaddr]"
			"                  prints the value of the given register(s)/stack\n"
		},
		
		{"quit", Quit, "quits the debugger",
			"  quit            quits the debugger\n"
		},

		{"run", Run, "runs the program",
			"  run ([n ins])   runs the specified number of instructions, or forever if nothing is specified.\n"},

		{"step", Step, "steps one instruction in the program", 
			"  step            steps one instruction in the program.\n"},

		{"where", Where, "shows the current line of code",
			"  where           shows the current line of code from the source file.\n"},

		};

	me->commands = commands;
	me->nCommands = sizeof(commands) / sizeof(Command);	

	commands[0].fun = Help;

	if(me->showNextIns) Where(me, 0, NULL);
	me->showNextIns = false;

	int argc = 0;
	char* argv[128];

	char* pch = strtok(input, " ,");

	if(!pch) return;

	do{
		argv[argc++] = strdup(pch);
	}while((pch = strtok(NULL, " ,")));

	int found = 0;
	int index = 0;

	for(int i = 0; i < me->nCommands; i++){
		if(!strncmp(argv[0], commands[i].cmd, strlen(argv[0]))){
			index = i;
			found++;
		}
	}

	if(found == 0) DPRINT("%s - no such command, type help for more information\n", argv[0]);

	else if(found > 1){
		DPRINT("'%s' is ambiguous, what did you mean? ", argv[0]);
		for(int i = 0; i < me->nCommands; i++)
			if(!strncmp(argv[0], commands[i].cmd, strlen(argv[0])))
				DPRINT("%s ", commands[i].cmd);
		DPRINT("\n");
	}

	else{
		commands[index].fun(me, argc, argv);
	}


	for(int i = 0; i < argc; i++) free(argv[i]);
}

bool Debug_Inspector(Cpu* cpu, void* vme)
{
	Debug* me = vme;

	if(me->runInstructions == 0)
		return false;

	if(me->runInstructions > 0)
		me->runInstructions--;

	me->insExec++;
	return true;
}

Debug* Debug_Create(Cpu* cpu, Mem* mem)
{
	Debug* me = calloc(1, sizeof(Debug));

	Cpu_SetInspector(cpu, Debug_Inspector, me);
	me->cpu = cpu;
	me->mem = mem;

	Vector_Init(me->sourceFiles, SourceFilePtr);
	Vector_Init(me->debugSymbols, DebugSymbol);
	Vector_Init(me->breakPoints, BreakPoint);

	me->Print = Debug_DefaultPrintHandler;

	return me;
}

SourceFile* AddSourceFile(Debug* me, const char* filename){
	// If it's already added, return the old one
	SourceFile** it;
	Vector_ForEach(me->sourceFiles, it){
		if(!strcmp((*it)->filename, filename))
			return *it;
	}

	// Create a new SourceFile structure and read the 
	// entire file's contents into it

	SourceFile* sf = calloc(1, sizeof(SourceFile));

	sf->filename = strdup(filename);

	Vector_Init(sf->file, char);
	Vector_Init(sf->lineIndices, int);

	FILE* f = fopen(filename, "r");
	LAssert(f, "could not locate source file: %s", filename);

	bool done = false;
	int lineIndex = 0;
	int lineLength = 0;

	while(!done){
		while(!done){
			int c = fgetc(f);
			if(c == EOF){
				done = true;
				break;
			}
			
			lineLength++;

			if(c == '\n' || c == '\r'){
				Vector_Add(sf->file, '\0');
				break;
			}

			Vector_Add(sf->file, c);
		}

		Vector_Add(sf->lineIndices, lineIndex);
		lineIndex += lineLength;
		lineLength = 0;
	}

	fclose(f);

	Vector_Add(me->sourceFiles, sf);

	LogI("Loaded source file from: %s", filename);
	return sf;
}


bool Debug_LoadSymbols(Debug* me, const char* filename)
{
	FILE* f = fopen(filename, "r");
	if(!f){
		LogW("debug symbols not loaded, could not open file: '%s'", filename);
		return false;
	}

	while(true){
		char buffer[2048];
		char sourcename[512];
		if(fgets(buffer, sizeof(buffer), f) == NULL) break;

		unsigned int addr, length, line;
		char t[32][512];

		// Because fuck it
		int ret = sscanf(buffer, 
			"%x %x %u %s"
			"%s %s %s %s %s %s %s %s" "%s %s %s %s %s %s %s %s"
			"%s %s %s %s %s %s %s %s" "%s %s %s %s %s %s %s %s",
			&addr, &length, &line, sourcename,
			t[ 0], t[ 1], t[ 2], t[ 3], t[ 4], t[ 5], t[ 6], t[ 7], 
			t[ 8], t[ 9], t[10], t[11], t[12], t[13], t[14], t[15],
			t[16], t[17], t[18], t[19], t[20], t[21], t[22], t[23],
			t[24], t[25], t[26], t[27], t[28], t[29], t[30], t[31]
			);

		LAssert(ret >= 4, "could not parse debug file line in %s", filename);

		//LogI("line: %d", line);
		DebugSymbol s = {addr, length, line, AddSourceFile(me, sourcename)};

		Vector_Init(s.items, CharPtr);
		
		for(int i = 0; i < ret - 4; i++){
			Vector_Add(s.items, strdup(t[i]));
		}

		Vector_Add(me->debugSymbols, s);

		//puts(buffer);
	}

	/*SourceFile** it;
	Vector_ForEach(me->sourceFiles, it){
		LogI("file: %s", (*it)->filename);
		int* it2;
		Vector_ForEach((*it)->lineIndices, it2){
			puts((*it)->file.elems + *it2);
		}
	}*/

	fclose(f);

	LogI("loaded debug symbols from %s", filename);

	return true;
}

void Debug_Destroy(Debug** me)
{
	free(*me);
	*me = NULL;
}
