#define LDEBUG

#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>

#ifdef __linux
#include <signal.h>
#endif

#include "utils.h"
#include "emu.h"
#include "apu.h"
#include "hyper.h"
#include "debug.h"
#include "SDL_picofont.h"

#include "log.h"

#include "debugwidget.h"

struct Emu {
	Settings* settings;
	Apu* apu;
	DebugWidget* debugWidget;
};

Emu* Emu_Create(Settings* settings)
{
	Emu* me = calloc(1, sizeof(Emu));
	me->settings = settings;
	LAssert(me, "could not allocate memory for the emulator class");
	return me;
}


//#include <math.h>

void Emu_AudioCallback(void* vMe, Uint8 *stream, int len)
{
	Emu* me = (Emu*)vMe;
	Apu_FetchAudio(me->apu, (int16_t*)stream, len / 4);

	/*static float t = 0;

	for(int i = 0; i < len / 4; i++){
		t += .1;
		((int16_t*)stream)[i * 2] = sinf(t) * 8192.0f;
		((int16_t*)stream)[i * 2 + 1] = sinf(t) * 8192.0f;
	}*/
}

void Emu_InitAudio(Emu* me){
	LogV("inititalizing audio");

	SDL_AudioSpec fmt;

	fmt.freq = 48000;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 1024;
	fmt.callback = Emu_AudioCallback;
	fmt.userdata = me;

	LAssert(SDL_OpenAudio(&fmt, NULL) == 0, "failed to open audio device: %s", SDL_GetError());

	SDL_PauseAudio(0);
}

void Emu_ScaleBlit(Emu* me, SDL_Surface* target, SDL_Surface* src, int mult)
{
	int x = 0;
	#define EBS_BEFORE \
		uint8_t* tp = target->pixels, *sp = src->pixels;\
		for(int y = 0; y < src->h; y++){\
			for(int j = 0; j < mult; j++){\
				for(x = 0; x < src->w; x++){\
					for(int i = 0; i < mult; i++){

	#define EBS_AFTER \
				}\
				sp += 3;\
			}\
			sp -= (src->pitch);\
			tp += target->pitch - x * mult * 3;\
		}\
		sp += src->pitch;\
	}

	if(target->format->Rmask == 0xff0000){
		// RGB24
		EBS_BEFORE

		*(tp++) = *(sp + 2);
		*(tp++) = *(sp + 1);
		*(tp++) = *sp;
		
		EBS_AFTER
	}else{
		// BGR24 (I hope)
		EBS_BEFORE

		*(tp++) = *sp;
		*(tp++) = *(sp + 1);
		*(tp++) = *(sp + 2);
		
		EBS_AFTER
	}
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

	SDL_Surface* screen = SDL_SetVideoMode(wUse, hUse, 24, SDL_SWSURFACE);

	// Virtual display pixels
	uint8_t* px = calloc(1, w * h * 3);

	SDL_Surface* origSurface = SDL_CreateRGBSurfaceFrom(px, w, h, 24, w * 3, 0xff0000, 0xff00, 0xff, 0);

	bool done = false;

	HyperMachine* hm = HM_Create(w, h, 1000.0 / 16.0, px, settings->debugFile != NULL, settings->freq);
	LAssert(HM_LoadRom(hm, settings->file, settings->debugFile), "bailing");

	me->apu = HM_GetApu(hm);

	if(settings->debugFile){
		me->debugWidget = DW_Create(hm);
	}
	
	int t = 0, tSpent = 0, frameCount = 0;

	Emu_InitAudio(me);
	
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

		
		t = SDL_GetTicks(); 
		HM_ProcessFrame(hm);
		tSpent += SDL_GetTicks() - t;

		int nFrames = 60 * 5;
		if(frameCount++ >= nFrames){
			LogV("spent %d ms for the last %d frames, or %.2f ms/frame", tSpent, nFrames, (double)tSpent / (double)nFrames);
			frameCount = 0;
			tSpent = 0;
		}

		if(videoFile){
			if(settings->numRecFrames == 0 || frame++ < settings->numRecFrames)
				fwrite(px, w * h * 3, 1, videoFile);
		}

		if(me->debugWidget)
			DW_Draw(me->debugWidget, screen, w * mult, h * mult);

		Emu_ScaleBlit(me, screen, origSurface, mult);

		//DrawText(screen, 10, 10, white, "frametime: %d", frameTime);

		SDL_Flip(screen);

		int frameTime = SDL_GetTicks() - timer;
		if(frameTime < 16){
			SDL_Delay(16 - frameTime);
		}
	}

	if(videoFile)
		fclose(videoFile);

	return 0;
}
