#include "belief.h"
#include "factor.h"
#include "load.h"
#include "util.h"

int main(int argc, const char **argv)
{
	if (argc != 2)
		die("Usage: %s file-with-ratings.csv\n", argv[0]);

	for (int it = 0; it < 100; it++)
		drand48();

	rating_t *ratings;
	int n = from_file(argv[1], &ratings);
	graph_t G;
	graph_from_edge_list(ratings, 1, &G);
	for (int it = 0; it < 10; it++) {
#ifdef DEBUG
		printf("=== Iteration %3d ===\n", it);
		dump_beliefs(&G);
#endif
		propagate(&G);
        get_beliefs(&G);
	}
	dump_graph(&G);
}
