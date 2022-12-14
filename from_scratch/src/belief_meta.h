#if OPTVARIANT == 2
#include "belief2.h"
#elif OPTVARIANT == 3
#include "belief3.h" //precompute msg products
#elif OPTVARIANT == 4
#include "belief_simpleUnroll.h" //unroll j loop factor 4
#elif OPTVARIANT == 5
#include "belief4.h" //unroll j loop + precompute msg products
#elif OPTVARIANT == 6
#include "belief3_vector.h" 
#elif OPTVARIANT == 7
#include "belief5.h" //Compact messages/node potentials/beliefs
#elif OPTVARIANT == 8
#include "belief6.h" //vectorization + precompute msg products
#elif OPTVARIANT == 9
#include "belief3_unroll_i4.h" //unroll i loop factor 4
#elif OPTVARIANT == 10
#include "belief3_unrolled.h"
#elif OPTVARIANT == 11
#include "belief_lowerbound.h"
#elif OPTVARIANT == 12
#include "belief_lowerbound2.h"
#elif OPTVARIANT == 13
#include "belief6_unroll16.h" //vectorization unrolled twice + precompute msg products
#elif OPTVARIANT == 14
#include "belief3_restrict.h"
#else
#include "belief.h"
#endif
