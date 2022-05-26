#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#define COMPACT_MESSAGE
#include "factor.h"

// Based on belief5.h, caculates a lower bound if we did (almost) no flops and just accessed the memory.
void propagate(graph_t *G) {
	// swap G->in and G->in_out
	auto *tmp = G->in;
	G->in = G->in_old;
	G->in_old = tmp;

	const auto n = G->n;
	const idx_t * __restrict__ off = G->off;
	const float_t * __restrict__ in = G->in;
	const float_t * __restrict__ in_old = G->in_old;
	const idx_t * __restrict__ Gout = G->out;
	const float_t * __restrict__ node_pot = G->node_pot;

	size_t start;
	size_t end = off[0];
	for (idx_t i = 0; i < n; i++) {
		start = end;
		end = off[i + 1];

		float_t pot_i1 = node_pot[i];
		float_t prod_tot0 = 1;

		for (auto k = start; k < end; k++)
			prod_tot0 += in_old[k];
		
		prod_tot0 *= pot_i1; // to force an use of pot_i1
		for (auto j = start; j < end; j++) {
			float_t val1 = in_old[j];
			float_t *_out = (float_t *)(in + Gout[j]);
			*_out = prod_tot0 * val1;
		}
	}

}

/* Calculates the current node beliefs given current messages. */
void get_beliefs(graph_t *G) {
	const auto n = G->n;
	const auto off = G->off;
	const auto in = G->in;
	const auto belief = G->belief;

	for (idx_t i = 0; i < n; i++) {
		float_t b0 = 0;
		for (idx_t j = off[i]; j < off[i + 1]; j++)
			b0 += in[j];

		belief[i] = b0;
	}
}

#endif
