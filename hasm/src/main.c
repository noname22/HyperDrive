#include "cvector.h"
#include "common.h"
#include "hasm.h"

int logLevel;

int main(int argc, char** argv)
{
	logLevel = 2;

	int atFile = 0;
	int romSize = 1024 * 1024 * 16;
	unsigned addr = 0;
	bool debugSymbols = false;

	const char* files[2] = {NULL, NULL};
	const char* usage = "usage: %s (-vX | -h | -sX | -d | -eX) [hasm file] [out binary]";

	for(int i = 1; i < argc; i++){
		char* v = argv[i];
		if(STARTSWITH(v, '-')){
			if(!strcmp(v, "-h")){
				LogI(usage, argv[0]);
				LogI(" ");
				LogI("Available flags:");
				LogI("  -vX   set log level, where X is [0-5] - default: 2");
				LogI("  -sX   set assembly start address [0-FFFF] - default 0");
				LogI("  -h    show this help message");
				LogI("  -d    generate debug symbols");
				return 0;
			}
			else if(sscanf(v, "-v%d", &logLevel) == 1){}
			else if(sscanf(v, "-s%x", &addr) == 1){}
			else if(!strcmp(v, "-d")){ debugSymbols = true; }
			else{
				LogF("No such flag: %s", v);
				return 1;
			}
		}else{
			LAssert(atFile < 2, "Please specify exactly one input file and one output file");
			files[atFile++] = v;
		}
	}

	LAssert(argc >= 3 && files[0] && files[1], usage, argv[0]);
	
	// Allocate 16 MB ROM
	uint8_t* ram = calloc(1, romSize);

	Hasm* d = Hasm_Create();
	
	if(debugSymbols){
		char tmp[4096];
		sprintf(tmp, "%s.dbg", files[1]);
		d->debugFile = fopen(tmp, "w");
		LogV("Opening debug file: %s", tmp);
		LAssert(d->debugFile, "could not open file: %s", tmp);
	}

	uint32_t len = Hasm_Assemble(d, files[0], ram, addr, romSize - 1);

	if(d->debugFile) fclose(d->debugFile);

	Hasm_Destroy(&d);

	if(logLevel == 0) DumpRam(ram, len - 1);

	LogV("Writing to: %s", files[1]);
	WriteFile(ram, len - 1, files[1]);

	free(ram);
	return 0;
}
