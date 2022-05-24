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
#else
#include "belief.h"
#endif
#include "factor.h"
#include "load.h"
#include "util.h"
#include "../../measurement_utils/tsc_x86.h"

#define REP 10

int main(int argc, const char **argv)
{
	if (argc != 2)
		die("Usage: %s file-with-ratings.csv\n", argv[0]);

    int target_uid = 1;
    int iterations = 10;

    myInt64 start_gbuild, end_gbuild, start_prop, end_prop;
    double total = 0;
    double total_gbuild = 0;
    double total_prop = 0;
	for (int it = 0; it < 3000; it++) // warm up the CPU
		drand48();

	rating_t *ratings;
	int n = from_file(argv[1], &ratings);
    graph_t G;

    for (int i = 0; i < REP; i++) {
        start_gbuild = start_tsc();
        graph_from_edge_list(ratings, target_uid, &G);
        end_gbuild = stop_tsc(start_gbuild);
        total_gbuild += (double) end_gbuild;
        
        start_prop = start_tsc();
        for (int it = 0; it < iterations; it++) {
#ifdef DEBUG
            printf("=== Iteration %3d ===\n", it);
            dump_beliefs(&G);
#endif
		    propagate(&G);
            get_beliefs(&G);
	    }
        end_prop = stop_tsc(start_prop);
        total_prop += (double) end_prop;

        if (i != REP-1) {
			graph_destroy(&G);
        }
    }
#ifdef DEBUG
	dump_graph(&G);
#endif
    total_gbuild /= REP;
    total_prop /= REP;
    total = total_gbuild + total_prop;


    unsigned int flops_gbuild = 0;
    unsigned int flops_prop = 0;
    unsigned int flops_belief = 0;
    unsigned int total_flops;
    // count number of flops in graph_from_edge_list (get_user_mean, get_user_stddev and z-score calcuations)
    // get_user_mean in filter_below_mean: n ratings + number of users
    flops_gbuild += (n + ratings[n-1].user);
    // get_user_mean, get_user_stddev and node potential calculations for target user:
    unsigned int cnt = 0;
    for (rating_t *p = ratings; p->user == ratings->user; p++) //number of items by targetuser rated
        cnt++;
    flops_gbuild += (9*cnt + 3); //9 flops for each item + 3 by 
    // printf("flops_gbuild: %d\n", flops_gbuild);

    // number of flops in propagate
    for (int i = 0; i < G.n; i++) {
		for (int j = G.off[i]; j < G.off[i + 1]; j++) {
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
            flops_prop++; //float_t s = m->L + m->D in normalise_msg(out), conservative estimate (don't consider s >= 1e-6 resulting to one more flop)
        }
    }
    flops_prop *= iterations;

    // number of flops in get_beliefs
    for (int i = 0; i < G.n; i++) {
		for (int c = 0; c < 2; c++) {
			for (int j = G.off[i]; j < G.off[i + 1]; j++)
				flops_belief++;
		}

		flops_belief++; //normalise_msg(&G->belief[i])
	}
    flops_belief *= iterations;
    // printf("flops_belief: %d\n", flops_belief);

    total_flops = flops_belief + flops_gbuild + flops_prop;
    
	graph_destroy(&G);
	// dump_graph(&G);
    // n (number of vertices), total_cycle, total_flops, gbuild_cycle, prop_cycle, gbuild_flops, prop_flops, bel_flops\n
    printf("%zu, %f, %u, %f, %f, %u, %u, %u\n", G.n, total, total_flops, total_gbuild, total_prop, flops_gbuild, flops_prop, flops_belief);

    free(ratings);
}
