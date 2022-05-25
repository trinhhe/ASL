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
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif
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
			float_t out0 = 0;
			float_t out1 = 0;

			out0 += PROP_00 * pot_i0 * prod0;
			out1 += PROP_01 * pot_i0 * prod0;
			out0 += PROP_10 * pot_i1 * prod1;
			out1 += PROP_11 * pot_i1 * prod1;

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif
			float_t a = out0 + out1;
			if (fabs(a) < EPS) {
				_out[0] = .5;
				_out[1] = .5;
			} else {
				_out[0] = out0 / a;
				_out[1] = out1 / a;
			}
		}
	}
}

#include "get_belief_stock.h"
#endif
