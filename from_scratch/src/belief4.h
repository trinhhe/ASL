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

		float_t prod_tot0 = 1;
		float_t prod_tot1 = 1;
        
        size_t end = off[i+1];
        
		for (int k = off[i]; k < end; k++) {
			prod_tot0 *= ((float_t *)&in_old[k])[0];
			prod_tot1 *= ((float_t *)&in_old[k])[1];
		}
		float_t glob00 = PROP_00 * pot_i0 * prod_tot0;
		float_t glob01 = PROP_01 * pot_i0 * prod_tot0;
		float_t glob10 = PROP_10 * pot_i1 * prod_tot1;
		float_t glob11 = PROP_11 * pot_i1 * prod_tot1;

		for (int j = off[i]; j < end-3; j+=4) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0a = ((float_t *)&in_old[j])[0];
			float_t val1a = ((float_t *)&in_old[j])[1];
			float_t *_out_a = (float_t *)(in + Gout[j]);
			float_t out0a = 0;
			float_t out1a = 0;

            float_t val0b = ((float_t *)&in_old[j+1])[0];
			float_t val1b = ((float_t *)&in_old[j+1])[1];
			float_t *_out_b = (float_t *)(in + Gout[j+1]);
			float_t out0b = 0;
			float_t out1b = 0;

            float_t val0c = ((float_t *)&in_old[j+2])[0];
			float_t val1c = ((float_t *)&in_old[j+2])[1];
			float_t *_out_c = (float_t *)(in + Gout[j+2]);
			float_t out0c = 0;
			float_t out1c = 0;

            float_t val0d = ((float_t *)&in_old[j+3])[0];
			float_t val1d = ((float_t *)&in_old[j+3])[1];
			float_t *_out_d = (float_t *)(in + Gout[j+3]);
			float_t out0d = 0;
			float_t out1d = 0;

			out0a += glob00 / val0a;
			out1a += glob01 / val0a;
			out0a += glob10 / val1a;
			out1a += glob11 / val1a;

            out0b += glob00 / val0b;
			out1b += glob01 / val0b;
			out0b += glob10 / val1b;
			out1b += glob11 / val1b;

            out0c += glob00 / val0c;
			out1c += glob01 / val0c;
			out0c += glob10 / val1c;
			out1c += glob11 / val1c;

            out0d += glob00 / val0d;
			out1d += glob01 / val0d;
			out0d += glob10 / val1d;
			out1d += glob11 / val1d;

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif
			float_t a = out0a + out1a;
            float_t b = out0b + out1b;
            float_t c = out0c + out1c;
            float_t d = out0d + out1d;
			
            _out_a[0] = out0a / a;
            _out_a[1] = out1a / a;

            _out_b[0] = out0b / b;
            _out_b[1] = out1b / b;

            _out_c[0] = out0c / c;
            _out_c[1] = out1c / c;

            _out_d[0] = out0d / d;
            _out_d[1] = out1d / d;	
		}
        //leftover loop
        for (int j = max(0, end - 3); j < end; j++){ 
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0 = ((float_t *)&in_old[j])[0];
			float_t val1 = ((float_t *)&in_old[j])[1];
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
            _out[0] = out0 / a;
            _out[1] = out1 / a;
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
		for (int c = 0; c < 2; c++) {
			float_t b = 1;
			for (int j = off[i]; j < off[i + 1]; j++)
				b *= ((float_t *)&in[j])[c];
			((float_t *)&belief[i])[c] = b;
		}

		normalise_msg(&belief[i]);
	}
}

#endif

