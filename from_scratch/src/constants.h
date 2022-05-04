#ifndef BELIEF_CONSTANTS_H
#define BELIEF_CONSTANTS_H
#include "rating.h"
const float_t NORMALISE_P = 3;
const float_t ALPHA = 0.001; // TODO: lower, currently even with such a large ALPHA it does nothing
const float_t PROP_EQUAL = (float_t)0.5 + ALPHA;
const float_t PROP_UNEQUAL = (float_t)0.5 - ALPHA;
const float_t PROP_MATRIX[2][2] = {
	{PROP_EQUAL, PROP_UNEQUAL},
	{PROP_UNEQUAL, PROP_EQUAL}
};
#endif
