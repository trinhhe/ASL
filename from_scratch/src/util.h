#ifndef BELIEF_UTIL_H
#define BELIEF_UTIL_H
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void die(const char fmt[], ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

static int max(int a, int b) { return a > b ? a : b; }
#endif
