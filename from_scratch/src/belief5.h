#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include "factor2.h"

// Based on belief3.h
/* Do one step of belief propagation. */
void propagate(graph_t *G) {
	// swap G->in and G->in_out
	auto *tmp = G->in;
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

		float_t prod_tot0 = 1;
		float_t prod_tot1 = 1;
		for (int k = off[i]; k < off[i + 1]; k++) {
			prod_tot0 *= 1-in_old[k];
			prod_tot1 *= in_old[k];
		}
		float_t glob00 = PROP_00 * pot_i0 * prod_tot0;
		float_t glob01 = PROP_01 * pot_i0 * prod_tot0;
		float_t glob10 = PROP_10 * pot_i1 * prod_tot1;
		float_t glob11 = PROP_11 * pot_i1 * prod_tot1;

		for (int j = off[i]; j < off[i + 1]; j++) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0 = 1-in_old[j];
			float_t val1 = in_old[j];
			float_t *_out = (float_t *)(in + Gout[j]);
			float_t out0 = 0;
			float_t out1 = 0;

			out0 += glob00 / val0;
			out1 += glob01 / val0;
			out0 += glob10 / val1;
			out1 += glob11 / val1;

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif
			float_t a = out0 + out1;
			if (fabs(a) < 1e-6) {
				_out[0] = .5;
			} else {
				_out[0] = out1 / a;
			}
		}
	}

}

/* Calculates the current node beliefs given current messages. */
void get_beliefs(graph_t *G) {
	const auto n = G->n;
	const auto off = G->off;
	const auto in = G->in;
	const auto belief = G->belief;

	for (int i = 0; i < n; i++) {
		float_t b0 = 0;
		float_t b1 = 1;
		for (int j = off[i]; j < off[i + 1]; j++){
			b0 *= 1-in[j];
			b1 *= in[j];
		}
		((float_t *)&belief[i])[1] = b0;
		((float_t *)&belief[i])[1] = b1;

		// normalize
		float_t s = belief[i].L + belief[i].D;
		if (s < 1e-6) {
			belief[i].L = belief[i].D = .5;
		} else {
			belief[i].L /= s;
			belief[i].D /= s;
		}
	}
}

#endif

