//#define LDEBUG

#include <string.h>
#include <stdbool.h>

#include "log.h"
#include "emu.h"

int logLevel = 0;

int main(int argc, char** argv)
{
	const char* usage = "usage: %s (-vX | -d) [hyperdrive ROM file]";
	LAssert(argc >= 2, usage, argv[0]);

	Settings settings;
	memset(&settings, 0, sizeof(Settings));

	settings.freq = 8000000;

	bool debugging = false;

	logLevel = 2;
	
	int numFiles = 0;
	float fFreq;
	char videoFile[1024] = {0};

	for(int i = 1; i < argc; i++){
		char* v = argv[i];
		if(v[0] == '-'){
			if(!strcmp(v, "-h")){
				LogI(usage, argv[0]);
				LogI(" ");
				LogI("Available flags:");
				LogI("  -vX   set log level, where X is [0-5] - default: 2");
				LogI("  -d    start with debugger");
				LogI("  -rV   record video (raw RGB888, 320x240), where V is the output file");
				LogI("  -lF   limit number of frames to record to F - default: 0 = infinite");
				LogI("  -fF   interpret at frequency F in MHz - default 8.0 MHz");
				return 0;
			}
			else if(sscanf(v, "-f%f", &fFreq) == 1){ settings.freq = (int)(fFreq * 1000000.0f); }
			else if(sscanf(v, "-v%d", &logLevel) == 1){}
			else if(sscanf(v, "-l%d", &settings.numRecFrames) == 1){}
			else if(sscanf(v, "-r%1023s", videoFile) == 1){ settings.videoFile = videoFile; }
			else if(!strcmp(v, "-d")){ debugging = true; }
			else{
				LogF("No such flag: %s", v);
				return 1;
			}
		}else{
			numFiles++;
			settings.file = v;
		}
	}

	LAssert(numFiles == 1, "Please specify one file to run");

	if(debugging){	
		char tmp[512] = {0};
		strcat(tmp, settings.file);
		strcat(tmp, ".dbg");
		settings.debugFile = tmp;
	}

	Emu* emu = Emu_Create(&settings);
	return Emu_Run(emu);
}
