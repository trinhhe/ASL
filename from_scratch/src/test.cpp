#include "belief_meta.h"
// edit belief_meta.h to add optimisation variants 
#include "factor.h"
#include "load.h"
#include "util.h"

void calculate_graph_and_beliefs(rating_t *ratings, int target_uid, graph_t *G) {
    int iterations = 10;
	graph_from_edge_list(ratings, target_uid, G);
	for (int it = 0; it < iterations; it++) {
#ifdef DEBUG
		printf("=== Iteration %3d ===\n", it);
		dump_beliefs(G);
#endif
		propagate(G);
	}

	get_beliefs(G);
#ifdef DEBUG
	dump_graph(G);
#endif
}

void dump_nontrivial_beliefs(graph_t *G) {
	for (int v = G->tr.m_lo; v < G->tr.m_hi; v++) {
		float_t b = *((float_t*)&(G->belief[v]));
		if (fabs(b - .5) < 1e-6)
			continue;
		printf("%d %.3f\n", v, b);
	}
}

int main(int argc, const char **argv) {
	if (argc != 2)
		die("Usage: %s file-with-ratings.csv\n", argv[0]);

	rating_t *ratings;
	int n = from_file(argv[1], &ratings);
	int max_uid = ratings[n - 1].user;
	// we assume uids are continuous and starting from 1
	
	int samples = 10;

    graph_t G;
	for (int i = 0; i < samples; i++) {
		int target_uid = ((double)i / samples) * (max_uid - 1);
		target_uid = 1 + (target_uid % max_uid); // just to be sure
		printf("== Sample #%d: target_uid = %d ==\n", i, target_uid);
		calculate_graph_and_beliefs(ratings, target_uid, &G);
		dump_nontrivial_beliefs(&G);
	}
	graph_destroy(&G);

    free(ratings);
}
