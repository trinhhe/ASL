 #include "belief_meta.h"
// edit belief_meta.h to add optimisation variants 
#include "factor.h"
#include "load.h"
#include "util.h"
#include "../../measurement_utils/tsc_x86.h"

int REP = 5;
const long long MIN_CYCLES = 2000000000LL;

using ull = unsigned long long;

typedef struct {
	ull prop, belief, total;
} flop_count;

ull flops_gbuild = 0;

//original belief.h flop count
flop_count count_flops_old(graph_t &G) {
	ull flops_prop = 0, flops_belief = 0;
    // number of flops in propagate
    for (int i = 0; i < G.n; i++) {
		for (int j = G.off[i]; j < G.off[i + 1]; j++) {
#ifdef GRAPH_PADDING
			if (G.out[j] == -1)
				break; // reached padding
#endif
            for (int c = 0; c < 2; c++) {
				for (int d = 0; d < 2; d++) {
                    for (int k = G.off[i]; k < G.off[i + 1]; k++) {
						if (k == j)
							continue;
						flops_prop++;  //prod *= ((float_t *)&G->in_old[k])[d];
					}
                    flops_prop += 3; //_out[c] += pot_i * pot_ij * prod;
                }
            }
            flops_prop+=3; //float_t s = m->L + m->D in normalise_msg(out), worst case estimate (consider only s >= EPS resulting to one more flop)
        }
    }
    

    // number of flops in get_beliefs
    for (int i = 0; i < G.n; i++) {
		for (int c = 0; c < 2; c++) {
			for (int j = G.off[i]; j < G.off[i + 1]; j++)
				flops_belief++;
		}

		flops_belief+=3; //normalise_msg(&G->belief[i])
	}

	return {flops_prop, flops_belief, flops_prop + flops_belief + flops_gbuild};
}

#define WITHOUT_COMPACT_MSG

flop_count count_flops_new(graph_t &G) {
	ull flops_prop = 0, flops_belief = 0;
    // number of flops in propagate
    for (int i = 0; i < G.n; i++) {
#ifndef WITHOUT_COMPACT_MSG
        flops_prop += 1;
#endif
        for (int k = G.off[i]; k < G.off[i+1]; k++) {
#ifdef WITHOUT_COMPACT_MSG
            flops_prop += 2;
#else
            flops_prop += 3;
#endif
        }
        flops_prop += 8; // four times globxx = PROP_xx * pot_ix * prod_totx
		for (int j = G.off[i]; j < G.off[i + 1]; j++) {
#ifdef GRAPH_PADDING
			if (G.out[j] == -1)
				break; // reached padding
#endif
#ifdef WITHOUT_COMPACT_MSG
            flops_prop += 8; //four times outx += globxx / valx
            flops_prop+=3; //normalize (worst case if fabs(a) >= EPS)
#else
            flops_prop += 9; //four times outx += globxx / valx
            flops_prop +=2; //normalize (worst case if fabs(a) >= EPS)
#endif
        }
    }
    

    // number of flops in get_beliefs
    for (int i = 0; i < G.n; i++) {
#ifdef WITHOUT_COMPACT_MSG
		for (int c = 0; c < 2; c++) {
			for (int j = G.off[i]; j < G.off[i + 1]; j++)
				flops_belief++;
		}

		flops_belief+=3; //normalise_msg(&G->belief[i])
#else
        for (int j = G.off[i]; j < G.off[i + 1]; j++)
            flops_belief+=3;
        flops_belief += 2; //normalise_msg(&G->belief[i])
#endif
	}
	return {flops_prop, flops_belief, 0};
}

flop_count adjust_flops(flop_count c, int iterations) {
	c.prop *= iterations;
	c.total = c.prop + c.belief + flops_gbuild;
	return c;
}

ull lower_bound_databytes(graph_t &G) {
    ull n = (ull) G.n;
    ull m = (ull) G.m;
    ull data_bytes = 0;
    data_bytes +=  4 * n; //G.off (size_t/unsigned int)
    data_bytes +=  4 * m; //G.out (size_t/unsigned int)
#ifdef COMPACT_MESSAGE
    data_bytes += 2 * 4 * m; //G.in & G.in_old (float)
    data_bytes += 2 * 4 * n; //G.node_pot & G.belief (float)
#else
    data_bytes += 2 * 8 * m; //G.in & G.in_old (float)
    data_bytes += 2 * 8 * n; //G.node_pot & G.belief (float)
#endif
    return data_bytes;
}

int main(int argc, const char **argv)
{
	if (argc != 2)
		die("Usage: %s file-with-ratings.csv\n", argv[0]);

    int target_uid = 1;
    int iterations = 10;

    myInt64 start_gbuild, end_gbuild, start_prop, end_prop, start_belief, end_belief;
    double total = 0;
    double total_gbuild = 0;
    double total_prop = 0;
    double total_belief = 0;
	for (int it = 0; it < 3000; it++) // warm up the CPU
		drand48();

	rating_t *ratings;
	int n = from_file(argv[1], &ratings);
    graph_t G;

    for (int i = 0; i < REP; i++) {
        start_gbuild = start_tsc();
        graph_from_edge_list(ratings, target_uid, &G);
        end_gbuild = stop_tsc(start_gbuild);
		if (i)
			total_gbuild += (double) end_gbuild;
        
        start_prop = start_tsc();
        for (int it = 0; it < iterations; it++)
		    propagate(&G);
        end_prop = stop_tsc(start_prop);
		if (i)
			total_prop += (double) end_prop;

        start_belief = start_tsc();
        get_beliefs(&G);
        end_belief = stop_tsc(start_belief);
		if (i)
			total_belief += (double) end_belief;

		if (i == REP-1 && total_prop + total_gbuild <= MIN_CYCLES)
			REP++;
        if (i != REP-1) {
			graph_destroy(&G);
        }
    }

#ifdef DEBUG
	dump_graph(&G);
#endif
    total_gbuild /= REP-1;
    total_prop /= REP-1;
    total_belief /= REP-1;
    total = total_gbuild + total_prop + total_belief;


    // count number of flops in graph_from_edge_list (get_user_mean, get_user_stddev and z-score calcuations)
    // get_user_mean in filter_below_mean: n ratings + number of users
    flops_gbuild += (n + ratings[n-1].user);
    // get_user_mean, get_user_stddev and node potential calculations for target user:
    unsigned int cnt = 0;
    for (rating_t *p = ratings; p->user == ratings->user; p++) //number of items by targetuser rated
        cnt++;
    flops_gbuild += (9*cnt + 3); //9 flops for each item + 3 by 

	auto flops_old = adjust_flops(count_flops_old(G), iterations);
	auto flops_new = adjust_flops(count_flops_new(G), iterations);
    auto data_bytes = lower_bound_databytes(G);
    
	graph_destroy(&G);
	// dump_graph(&G);
    // n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, gbuild_flops, prop_flops, bel_flops\n
    printf("%u, %f, %llu, %f, %f, %f, %llu, %llu, %llu, %d, %llu, %llu, %llu, %llu\n", (int)G.n, total, flops_new.total, total_gbuild, total_prop, total_belief, flops_gbuild, flops_new.prop, flops_new.belief, iterations, flops_old.prop, flops_old.belief, flops_old.total, data_bytes);

    free(ratings);
}
