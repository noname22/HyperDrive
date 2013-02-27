#include "utils.h"
#include <stdarg.h>
#include "SDL_picofont.h"
#include "log.h"

void DrawText(SDL_Surface* target, int x, int y, SDL_Color color, const char* format, ...)
{
	char buffer[1024];
	memset(buffer, '\0', sizeof(buffer));

	va_list fmtargs;
	va_start(fmtargs, format);

	vsnprintf(buffer, sizeof(buffer) - 1, format, fmtargs);

	va_end(fmtargs);

	DrawStr(target, x, y, color, buffer);
}

void DrawStr(SDL_Surface* target, int x, int y, SDL_Color color, const char* string)
{
	SDL_Surface* s = FNT_Render(string, color);
	SDL_Rect r = {x, y};
	SDL_BlitSurface(s, NULL, target, &r);
	SDL_FreeSurface(s);
}
