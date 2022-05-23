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

	for (int i = 0; i < G->n; i++) {
		for (int j = G->off[i]; j < G->off[i + 1]; j++) {
			// calculate the address where we should write our message to
			msg_t *out = G->in + G->out[j];
			float_t *_out = (float_t *)out;
			for (int c = 0; c < 2; c++) {
				_out[c] = 0;
				for (int d = 0; d < 2; d++) {
					float_t pot_i = ((float_t *)&G->node_pot[i])[d];
					float_t pot_ij = PROP_MATRIX[d][c];
					float_t prod = 1;
					for (int k = G->off[i]; k < G->off[i + 1]; k++) {
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

/* Calculates the current node beliefs given current messages. */
void get_beliefs(graph_t *G) {
	for (int i = 0; i < G->n; i++) {
		for (int c = 0; c < 2; c++) {
			float_t belief = 1;
			for (int j = G->off[i]; j < G->off[i + 1]; j++)
				belief *= ((float_t *)&G->in[j])[c];
			((float_t *)&G->belief[i])[c] = belief;
		}

		normalise_msg(&G->belief[i]);
	}
}

#endif
