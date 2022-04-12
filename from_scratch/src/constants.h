#ifndef BELIEF_CONSTANTS_H
#define BELIEF_CONSTANTS_H
#include "rating.h"
const float_t NORMALISE_P = 3;
const float_t ALPHA = 0.1;
const float_t PROP_MATRIX[2][2] = {
	{(float_t)0.5 + ALPHA, (float_t)0.5 - ALPHA},
	{(float_t)0.5 - ALPHA, (float_t)0.5 + ALPHA}
};
#endif
