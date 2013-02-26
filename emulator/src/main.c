//#define LDEBUG
#ifndef WIN32
#include <signal.h>
#endif

#include <SDL.h>

#include "log.h"

#include <stdbool.h>
#include <stdlib.h>

#include "hyper.h"

int logLevel = 0;

typedef struct {
	int freq;
	const char* file;
	const char* debugFile;
} Settings;

int Start(Settings* settings)
{
	SDL_Init(SDL_INIT_EVERYTHING);

        // Disable ctrl-c catching on linux
	#ifndef WIN32
        signal(SIGINT, SIG_DFL);
	#endif

	const int w = 320, h = 240, mult = 3;

	SDL_Surface* screen = SDL_SetVideoMode(w * mult, h * mult, 0, SDL_SWSURFACE);

	// Virtual display pixels
	uint8_t* px = calloc(1, w * h * 3);

	// Upscaled display
	uint8_t* spx = calloc(1, w * h * mult * mult * 3);
	SDL_Surface* scaleSurface = SDL_CreateRGBSurfaceFrom(spx, w * mult, h * mult, 24, w * 3 * mult, 0xff0000, 0xff00, 0xff, 0);
	//SDL_Surface* origSurface = SDL_CreateRGBSurfaceFrom(px, w, h, 24, w * 3, 0xff0000, 0xff00, 0xff, 0);

	bool done = false;

	HyperMachine* hm = HM_Create(w, h, px, settings->debugFile != NULL, settings->freq);
	LAssert(HM_LoadRom(hm, settings->file, settings->debugFile), "bailing");

	while(!done){
		int timer = SDL_GetTicks();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
				done = true;
			}
		}

		int t = SDL_GetTicks();
		HM_Tick(hm);
		LogD("HM_Tick: %d", SDL_GetTicks() - t);

		// scale mult times
		
		t = SDL_GetTicks();
		uint8_t* tp = spx, *sp = px;
		for(int y = 0; y < h; y++){
			for(int j = 0; j < mult; j++){
				for(int x = 0; x < w; x++){
					for(int i = 0; i < mult; i++){
						*(tp++) = *sp;
						*(tp++) = *(sp + 1);
						*(tp++) = *(sp + 2);
					}
					sp += 3;
				}
				sp -= w * 3;
			}
			sp += w * 3;
		}
		LogD("scale: %d", SDL_GetTicks() - t);

		//SDL_BlitSurface(origSurface, NULL, screen, NULL);
		SDL_BlitSurface(scaleSurface, NULL, screen, NULL);
		SDL_Flip(screen);

		int frameTime = SDL_GetTicks() - timer;

		if(frameTime < 16){
			LogD("frametime: %d", frameTime);
			SDL_Delay(16 - frameTime);
		}
	}

	return 0;
}

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

	for(int i = 1; i < argc; i++){
		char* v = argv[i];
		if(v[0] == '-'){
			if(!strcmp(v, "-h")){
				LogI(usage, argv[0]);
				LogI(" ");
				LogI("Available flags:");
				LogI("  -vX   set log level, where X is [0-5] - default: 2");
				LogI("  -d    start with debugger");
				LogI("  -fF   interpret at frequency F in MHz - default 8.0 MHz");
				return 0;
			}
			else if(sscanf(v, "-f%f", &fFreq) == 1){ settings.freq = (int)(fFreq * 1000000.0f); }
			else if(sscanf(v, "-v%d", &logLevel) == 1){}
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

	return Start(&settings);}

