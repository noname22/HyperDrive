#include "log.h"
#include "textbuffer.h"
#include "utils.h"
#include "cvector.h"

typedef Vector(char) CharVec;

struct TextBuffer {
	CharVec lines;
};

TextBuffer* TB_Create()
{
	TextBuffer* me = calloc(1, sizeof(TextBuffer));
	LAssert(me, "could not allocate memory for text buffer");
	Vector_Init(me->lines, char);
	Vector_Add(me->lines, 0);
	return me;
}

void TB_AddLine(TextBuffer* me, const char* text)
{
	Vector_Remove(me->lines, me->lines.count - 1, 1);
	for(int i = 0; i < strlen(text); i++){
		Vector_Add(me->lines, text[i]);
	}

	// Add 0 so that the string is null temrinated
	Vector_Add(me->lines, 0);
}

void TB_Draw(TextBuffer* me, SDL_Surface* target, int x, int y, int numLines)
{
	int at = 0, index = 0;
	SDL_Color color = {255, 255, 255, 255};

	for(int i = me->lines.count - 1; i >= 0; i--){
		if(me->lines.elems[i] == '\n'){
			if(++at == numLines){
				at--;
				break;
			}
		}
		index = i;
	}

	DrawStr(target, x, y + (numLines - at) * 8, color, me->lines.elems + index);
}
