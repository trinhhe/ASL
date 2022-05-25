#if OPTVARIANT == 2
#include "belief2.h"
#elif OPTVARIANT == 3
#include "belief3.h"
#elif OPTVARIANT == 4
#include "belief_simpleUnroll.h"
#elif OPTVARIANT == 5
#include "belief4.h"
#elif OPTVARIANT == 6
#include "belief3_vector.h"
#elif OPTVARIANT == 7
#include "belief5.h"
#else
#include "belief.h"
#endif
