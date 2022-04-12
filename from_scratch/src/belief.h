#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include "factor.h"

void propagate(graph_t *G) {
	msg_t *tmp = G->in;
	G->in = G->in_old;
	G->in_old = tmp;
	memset(G->in, 0, G->m * sizeof *G->in);

	for (int i = 0; i < G->m; i++)
		printf("%f %f ", G->in_old->D, G->in_old->L);
	printf("\n");

	for (int i = 0; i < G->n; i++) {
		for (int j = G->off[i]; j < G->off[i + 1]; j++) {
			msg_t *out = G->in + G->out[j];
			float_t *_out = (float_t *)out;
			for (int c = 0; c < 2; c++) {
				for (int d = 0; d < 2; d++) {
					float_t pot_i = ((float_t *)&G->node_pot[i])[d];
					float_t pot_ij = PROP_MATRIX[d][c];
					float_t prod = 1;
					for (int k = G->off[i]; k < G->off[i + 1]; k++) {
						if (k == j)
							continue;
						prod *= ((float_t *)&G->in_old[k])[d];
					}
					printf("%f*%f*%f=%f\n", pot_i, pot_ij, prod, pot_i * pot_ij * prod);
					_out[c] += pot_i * pot_ij * prod;
				}
			}
			printf("%f %f\n", out->D, out->L);
			normalise_msg(out);
		}
	}
}

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
