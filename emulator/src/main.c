#include <SDL.h>

#include "log.h"

#include <stdbool.h>
#include <stdlib.h>

#include "hyper.h"

int logLevel = 0;

int main(int argc, char** argv)
{
	LAssert(argc == 2, "usage: %s [rom]", argv[0]);

	LogI("== HYPERDRIVE ==");

	SDL_Init(SDL_INIT_EVERYTHING);

	const int w = 320, h = 240, mult = 3;

	SDL_Surface* screen = SDL_SetVideoMode(w * mult, h * mult, 0, SDL_SWSURFACE);

	// Virtual display pixels
	uint8_t* px = malloc(w * h * 3);

	// Upscaled display
	uint8_t* spx = malloc(w * h * mult * mult * 3);
	SDL_Surface* scaleSurface = SDL_CreateRGBSurfaceFrom(spx, w * mult, h * mult, 24, w * 3 * mult, 0xff0000, 0xff00, 0xff, 0);

	bool done = false;

	HyperMachine* hm = HM_Create(w, h, px);
	LAssert(HM_LoadRom(hm, argv[1]), "bailing");

	while(!done){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)){
				done = true;
			}
		}

		HM_Tick(hm);

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

		SDL_BlitSurface(scaleSurface, NULL, screen, NULL);
		SDL_Flip(screen);
		SDL_Delay(10);
	}

	return 0;
}
