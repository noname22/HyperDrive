#include "common.h"

static const char* dinsNames[] = DINSNAMES;
static const char* valNames[] = VALNAMES;

int logLevel;

void Disasm(uint8_t* ram, uint32_t start)
{
	LogV("starting disassembly at: 0x%08x", start);
	uint32_t pc = start;
	
	int end = 1024 * 1024 * 16;
	while(ram[--end] == 0);
	
	while(pc <= end){
		int hasRead = 0;
		uint8_t read8(){ hasRead++; return ram[pc++]; }

		uint32_t read16(){
			uint16_t ret = read8();
			ret |= read8() << 8;
			return ret;
		}

		uint32_t read32(){
			uint32_t ret = read8();
			ret |= read8() << 8;
			ret |= read8() << 16;
			ret |= read8() << 24;
			return ret;
		}

		bool hasNextWord[2];
		uint32_t nextWord[2];
		uint16_t pIns = read16();
		DVals v[2];

		int numVals = 2;

		DIns ins = pIns & 0xf;
		v[0] = (pIns >> 4) & 0x3f;
		v[1] = (pIns >> 10) & 0x3f;

		for(int i = 0; i < 2; i++){
			if((hasNextWord[i] = OpHasNextWord(v[i]))) nextWord[i] = read32();
		}
		
		if(ins == DI_NonBasic){
			numVals = 1;
			ins = v[0] + DINS_EXT_BASE;
			v[0] = v[1];
			hasNextWord[0] = hasNextWord[1];
			nextWord[0] = nextWord[1];

			hasNextWord[1] = false;
		}
		
		LAssert(ins < DINS_NUM, "illegal instruction: 0x%02x", ins);

		int n = printf("\t%s ", dinsNames[ins]);

		for(int i = 0; i < numVals; i++){
			LogD("nextWord: %02x", nextWord[i]);
			LogD("hasNExtWord: %d", hasNextWord[i]);
			char numStr[64] = {0};
			if(hasNextWord[i]) sprintf(numStr, "0x%02x", nextWord[i]);

			char str[64];
			n += printf("%s", StrReplace(str, valNames[v[i]], "NW", numStr));
			if(i == 0 && numVals == 2) n += printf(", ");
		}
		
		printf("%*s", 40 - n, "; ");
		printf("0x%08x | ", pc - hasRead);
		for(int i = 0; i < hasRead; i++) printf("%02x ", ram[pc - hasRead + i]);

		printf("\n");
	}
}

int main(int argc, char** argv)
{
	const char* usage = "usage: %s (-vX | -d) [dcpu-16 binary]";
	LAssert(argc >= 2, usage, argv[0]);

	logLevel = 2;
	
	const char* file = NULL;
	unsigned start = 0;
	int numFiles = 0;

	for(int i = 1; i < argc; i++){
		char* v = argv[i];
		if(v[0] == '-'){
			if(!strcmp(v, "-h")){
				LogI(usage, argv[0]);
				LogI(" ");
				LogI("Available flags:");
				LogI("  -vX   set log level, where X is [0-5] - default: 2");
				LogI("  -sX   start disassembly at address X - default 0");
				return 0;
			}
			else if(sscanf(v, "-v%d", &logLevel) == 1){}
			else if(sscanf(v, "-s0x%x", &start) || sscanf(v, "-s%d", &start) == 1){}
			else{
				LogF("No such flag: %s", v);
				return 1;
			}
		}else{
			numFiles++;
			file = v;
		}
	}
	
	LAssert(numFiles == 1, "Please specify one file to disassemble");

	// Allocate 16MB ROM/RAM
	uint8_t* ram = calloc(1, 1024 * 1024 * 16);

	ReadFile(ram, 1024 * 1024 * 16, file);
	Disasm(ram, start);

	free(ram);
}
