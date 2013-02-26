#!/usr/bin/tcc -run

#define LDEBUG

#include <math.h>
#include <stdlib.h>
#include "../../common/log.h"

int logLevel = 0;

int main(int argc, char** argv)
{
	LAssert(argc == 4, "usage: %s [steps] [amplitude] [outfile]", argv[0]);

	FILE* f = fopen(argv[3], "w");
	LAssert(f, "could not open output file");

	float t = 0, amp = atof(argv[2]);
	int n = atoi(argv[1]);

	for(int i = 0; i < n; i++){
		int32_t v = (sinf(t) * amp);
		LogD("%d", v);

		fputc(v & 0xff, f);
		fputc((v >> 8) & 0xff, f);
		fputc((v >> 16) & 0xff, f);
		fputc((v >> 24) & 0xff, f);

		t += ((float)M_PI * 2.0f) / (float)n;
	}

	fclose(f);
}
