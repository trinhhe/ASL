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
	// zero G->in out so that we can write our messages to it.
	memset(G->in, 0, G->m * sizeof *G->in);

	const auto n = G->n;
	const auto off = G->off;
	const auto in = G->in;
	const auto in_old = G->in_old;
	const auto Gout = G->out;
	const auto node_pot = G->node_pot;

	for (int i = 0; i < n; i++) {
		float_t pot_i0 = ((float_t *)&node_pot[i])[0];
		float_t pot_i1 = ((float_t *)&node_pot[i])[1];

		// idea: if no message is almost 0, we may just calculate prod* outside and inside just divide by the current in_old[j]
		for (int j = off[i]; j < off[i + 1]; j++) {
			float_t prod0 = 1;
			float_t prod1 = 1;

			// idea for more speedup: temporarily write in_old[k] = 1 in order to eliminate the if
			for (int k = off[i]; k < off[i + 1]; k++) {
				if (k != j) {
					prod0 *= ((float_t *)&in_old[k])[0];
					prod1 *= ((float_t *)&in_old[k])[1];
				}
			}

			msg_t *out = in + Gout[j];
			float_t *_out = (float_t *)out;
			float out0 = _out[0];
			float out1 = _out[1];

			out0 += PROP_EQUAL * pot_i0 * prod0;
			out1 += PROP_UNEQUAL * pot_i0 * prod0;
			out0 += PROP_UNEQUAL * pot_i1 * prod1;
			out1 += PROP_EQUAL * pot_i1 * prod1;

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif
			double a = out0 + out1;
			_out[0] = out0 / a;
			_out[1] = out1 / a;
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
