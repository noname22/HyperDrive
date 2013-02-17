#include "hyper.h"
#include "log.h"
#include <stdlib.h>

struct HyperMachine {
	int w, h;
	uint8_t* display;
};

HyperMachine* HM_Create(int w, int h, uint8_t* display)
{
	HyperMachine* me = calloc(1, sizeof(HyperMachine));
	LAssert(me, "could not allocate a machine");	

	me->display = display;
	me->w = w;
	me->h = h;

	return me;
}

void HM_Tick(HyperMachine* me)
{
	uint8_t* pill = me->display;
	for(int y = 0; y < me->h; y++){
		for(int x = 0; x < me->w * 3; x++){
			*(pill++) = rand() % 256;
		}
	}
}
