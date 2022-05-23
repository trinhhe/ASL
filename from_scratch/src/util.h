#ifndef BELIEF_UTIL_H
#define BELIEF_UTIL_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"

void *aligned_malloc(size_t size)
{
	void *p = nullptr;
	posix_memalign(&p, SIMD_ALIGN_BYTES, size);
	return p;
}

void *aligned_calloc(size_t nmemb, size_t size) {
	size_t n = nmemb * size;
	void *p = aligned_malloc(n);
	memset(p, 0, n);
	return p;
}

// Prints error message and exits
static void die(const char fmt[], ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

static int max(int a, int b) { return a > b ? a : b; }

// assumes multiple is a power of two
unsigned round_up(unsigned x, unsigned multiple) {
	auto ret = ((x - 1) & ~(multiple - 1)) + multiple;
	return ret;
}
#endif
