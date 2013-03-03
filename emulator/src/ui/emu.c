#define LDEBUG

#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>

#ifdef __linux
#include <signal.h>
#endif

#include "utils.h"
#include "emu.h"
#include "hyper.h"
#include "debug.h"
#include "SDL_picofont.h"

#include "log.h"

#include "debugwidget.h"

struct Emu {
	Settings* settings;
	DebugWidget* debugWidget;
};

Emu* Emu_Create(Settings* settings)
{
	Emu* me = calloc(1, sizeof(Emu));
	me->settings = settings;
	LAssert(me, "could not allocate memory for the emulator class");
	return me;
}

int Emu_Run(Emu* me)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_EnableUNICODE(1);

	Settings* settings = me->settings;

        // Disable ctrl-c catching on linux
	#ifdef __linux
        signal(SIGINT, SIG_DFL);
	#endif

	FILE* videoFile = NULL;
	int frame = 0;
	if(settings->videoFile){
		LAssert((videoFile = fopen(settings->videoFile, "w")), "could not open video file for writing");
		LogI("Recording video output to file: %s", settings->videoFile);
	}

	int w = 320, h = 240, mult = 3;
	int wUse = w * mult, hUse = h * mult;

	SDL_Color white = {255, 255, 255, 255};

	if(settings->debugFile){
		wUse = 1024;
		hUse = 768;
		mult = 2;
	}

	SDL_Surface* screen = SDL_SetVideoMode(wUse, hUse, 0, SDL_SWSURFACE);

	// Virtual display pixels
	uint8_t* px = calloc(1, w * h * 3);

	// Upscaled display
	uint8_t* spx = calloc(1, w * h * mult * mult * 3);
	SDL_Surface* scaleSurface = SDL_CreateRGBSurfaceFrom(spx, w * mult, h * mult, 24, w * 3 * mult, 0xff, 0xff00, 0xff0000, 0);
	//SDL_Surface* origSurface = SDL_CreateRGBSurfaceFrom(px, w, h, 24, w * 3, 0xff0000, 0xff00, 0xff, 0);

	bool done = false;

	HyperMachine* hm = HM_Create(w, h, px, settings->debugFile != NULL, settings->freq);
	LAssert(HM_LoadRom(hm, settings->file, settings->debugFile), "bailing");

	if(settings->debugFile){
		me->debugWidget = DW_Create(hm);
	}
	
	while(!done){
		int timer = SDL_GetTicks();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
				done = true;
			}

			if(me->debugWidget && DW_HandleEvent(me->debugWidget, &event))
				continue;

			if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP){
				switch(event.key.keysym.sym){
					case SDLK_UP:    HM_TriggerInput(hm, 0, HM_Up, event.type == SDL_KEYDOWN); break;
					case SDLK_DOWN:  HM_TriggerInput(hm, 0, HM_Down, event.type == SDL_KEYDOWN); break;
					case SDLK_LEFT:  HM_TriggerInput(hm, 0, HM_Left, event.type == SDL_KEYDOWN); break;
					case SDLK_RIGHT: HM_TriggerInput(hm, 0, HM_Right, event.type == SDL_KEYDOWN); break;
					default: break;
				}
			}
		}

		HM_Tick(hm);

		if(videoFile){
			if(settings->numRecFrames == 0 || frame++ < settings->numRecFrames)
				fwrite(px, w * h * 3, 1, videoFile);
		}

		// scale mult times
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

		if(settings->debugFile)
			SDL_FillRect(screen, NULL, 0x22222222);

		SDL_BlitSurface(scaleSurface, NULL, screen, NULL);

		int frameTime = SDL_GetTicks() - timer;
		DrawText(screen, 10, 10, white, "frametime: %d", frameTime);
	
		if(me->debugWidget)
			DW_Draw(me->debugWidget, screen, w * mult, h * mult);

		SDL_Flip(screen);

		if(frameTime < 16){
			SDL_Delay(16 - frameTime);
		}
	}

	if(videoFile)
		fclose(videoFile);

	return 0;
}
