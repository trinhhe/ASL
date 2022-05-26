#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include "factor.h"

/* Do one step of belief propagation. */
void propagate(graph_t *G) {
	// swap G->in and G->in_out
	msg_t *tmp = G->in;
	G->in = G->in_old;
	G->in_old = tmp;

	for (idx_t i = 0; i < G->n; i++) {
		for (idx_t j = G->off[i]; j < G->off[i + 1]; j++) {
#ifdef GRAPH_PADDING
			if (G->out[j] == -1)
				break; // reached padding
#endif
			// calculate the address where we should write our message to
			msg_t *out = G->in + G->out[j];
			float_t *_out = (float_t *)out;
			for (idx_t c = 0; c < 2; c++) {
				_out[c] = 0;
				for (idx_t d = 0; d < 2; d++) {
					float_t pot_i = ((float_t *)&G->node_pot[i])[d];
					float_t pot_ij = PROP_MATRIX[d][c];
					float_t prod = 1;
					for (idx_t k = G->off[i]; k < G->off[i + 1]; k++) {
						if (k == j)
							continue;
						prod *= ((float_t *)&G->in_old[k])[d];
					}
#ifdef DEBUG
					printf("%f*%f*%f=%f\n", pot_i, pot_ij, prod, pot_i * pot_ij * prod);
#endif
					_out[c] += pot_i * pot_ij * prod;
				}
			}
#ifdef DEBUG
			printf("unnorm: %f %f\n", out->D, out->L);
#endif
			normalise_msg(out); // TODO: did I understand their normalisation correctly?
		}
	}
}

#include "get_belief_stock.h"
#endif
