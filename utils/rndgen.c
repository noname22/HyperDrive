#!/usr/bin/tcc -run

#define LDEBUG

#include <math.h>
#include <stdlib.h>
#include "../common/log.h"

int logLevel = 0;

int main(int argc, char** argv)
{
	LAssert(argc == 5, "usage: %s [min] [max] [n] [outfile]", argv[0]);

	FILE* f = fopen(argv[4], "w");
	LAssert(f, "could not open output file");

	int n = atoi(argv[3]), min = atoi(argv[1]), max = atoi(argv[2]);

	for(int i = 0; i < n; i++){
		int32_t v = (rand() % (max - min)) + min;
		LogD("%d", v);

		fputc(v, f);
	}

	fclose(f);
}
