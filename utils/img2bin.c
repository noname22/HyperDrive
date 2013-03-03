#!/usr/bin/tcc -run -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/SDL -lSDL -lSDL_image
#include <SDL.h>
#include <SDL_image.h>

#include "../common/log.h"
#include <stdint.h>

int logLevel = 0;

int main(int argc, char** argv)
{
	LAssert(argc == 4, "usage: %s [source_image] [output.bin] [output.pal]", argv[0]);
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Surface* s = IMG_Load(argv[1]);
	LAssert(s, "could not load image");
	LAssert(s->format->BitsPerPixel == 8, "image must be 8 bpp");
	LAssert(s->format->palette, "image must have a palette");

	FILE* fPx = fopen(argv[2], "w");
	LAssert(fPx, "could not open pixel data file for writing: %s", argv[2]);
	FILE* fPal = fopen(argv[3], "w");
	LAssert(fPal, "could not open palette file for writing: %s", argv[3]);

	for(int y = 0; y < s->h; y++){
		fwrite((uint8_t*)s->pixels + y * s->pitch, s->w, 1, fPx);
	}

	SDL_Palette* p = s->format->palette;
	for(int c = 0; c < p->ncolors; c++){
		fputc(p->colors[c].r, fPal);
		fputc(p->colors[c].g, fPal);
		fputc(p->colors[c].b, fPal);
	}

	fclose(fPx);
	fclose(fPal);

	return 0;
}
