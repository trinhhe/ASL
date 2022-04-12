#include "load.h"
#include "factor.h"
#include "util.h"

int main(int argc, const char **argv)
{
	if (argc != 2)
		die("Usage: %s file-with-ratings.csv\n", argv[0]);

	rating_t *ratings;
	int n = from_file(argv[1], &ratings);
	graph_t G;
	graph_from_edge_list(ratings, 1, &G);
	dump_graph(&G);
}
