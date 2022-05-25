#ifndef BELIEF_CONSTANTS_H
#define BELIEF_CONSTANTS_H
#include "rating.h"
const float_t EPS = 1e-6;
const float_t NORMALISE_P = 3;
const float_t ALPHA = .3;
const float_t PROP_EQUAL = (float_t)0.5 + ALPHA;
const float_t PROP_UNEQUAL = (float_t)0.5 - ALPHA;
const float_t PROP_00 = .8;
const float_t PROP_01 = .2;
const float_t PROP_10 = .2;
const float_t PROP_11 = .8;
const float_t PROP_MATRIX[2][2] = {
	{PROP_00, PROP_01},
	{PROP_10, PROP_11}
};
const size_t SIMD_ALIGN_BYTES = 32;
#endif
