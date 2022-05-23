#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include "factor.h"

#ifdef GRAPH_PADDING
#error "FIXME: incompatible with -DGRAPH_PADDING yet"
#endif

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
		
        //unrolled, using a,b,c,d for j = 0,1,2,3...
        size_t end = off[i + 1];

        for (int j = off[i]; j < end - (end%4); j += 4) {
			float_t prod0a = 1;
			float_t prod1a = 1;
            float_t prod0b = 1;
			float_t prod1b = 1;
            float_t prod0c = 1;
			float_t prod1c = 1;
            float_t prod0d = 1;
			float_t prod1d = 1;

			// idea for more speedup: temporarily write in_old[k] = 1 in order to eliminate the if
			for (int k = off[i]; k < end; k++) {
				if (k != j) {
					prod0a *= ((float_t *)&in_old[k])[0];
					prod1a *= ((float_t *)&in_old[k])[1];
				}
                if (k != j+1) {
                    prod0b *= ((float_t *)&in_old[k])[0];
					prod1b *= ((float_t *)&in_old[k])[1];
                }
                if (k != j+2) {
                    prod0c *= ((float_t *)&in_old[k])[0];
					prod1c *= ((float_t *)&in_old[k])[1];
                }
                if (k != j+3) {
                    prod0d *= ((float_t *)&in_old[k])[0];
					prod1d *= ((float_t *)&in_old[k])[1];
                }
			}

			msg_t *out_a = in + Gout[j];
			float_t *_out_a = (float_t *)out_a;
			float out0a = _out_a[0];
			float out1a = _out_a[1];
            msg_t *out_b = in + Gout[j+1];
			float_t *_out_b = (float_t *)out_b;
			float out0b = _out_b[0];
			float out1b = _out_b[1];
            msg_t *out_c = in + Gout[j+2];
			float_t *_out_c = (float_t *)out_c;
			float out0c = _out_c[0];
			float out1c = _out_c[1];
            msg_t *out_d = in + Gout[j+3];
			float_t *_out_d = (float_t *)out_d;
			float out0d = _out_d[0];
			float out1d = _out_d[1];

			out0a += PROP_EQUAL * pot_i0 * prod0a;
			out1a += PROP_UNEQUAL * pot_i0 * prod0a;
			out0a += PROP_UNEQUAL * pot_i1 * prod1a;
			out1a += PROP_EQUAL * pot_i1 * prod1a;

            out0b += PROP_EQUAL * pot_i0 * prod0b;
			out1b += PROP_UNEQUAL * pot_i0 * prod0b;
			out0b += PROP_UNEQUAL * pot_i1 * prod1b;
			out1b += PROP_EQUAL * pot_i1 * prod1b;

            out0c += PROP_EQUAL * pot_i0 * prod0c;
			out1c += PROP_UNEQUAL * pot_i0 * prod0c;
			out0c += PROP_UNEQUAL * pot_i1 * prod1c;
			out1c += PROP_EQUAL * pot_i1 * prod1c;

            out0d += PROP_EQUAL * pot_i0 * prod0d;
			out1d += PROP_UNEQUAL * pot_i0 * prod0d;
			out0d += PROP_UNEQUAL * pot_i1 * prod1d;
			out1d += PROP_EQUAL * pot_i1 * prod1d;

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif
			double a = out0a + out1a;
			_out_a[0] = out0a / a;
			_out_a[1] = out1a / a;

            double b = out0b + out1b;
			_out_b[0] = out0b / b;
			_out_b[1] = out1b / b;

            double c = out0c + out1c;
			_out_c[0] = out0c / c;
			_out_c[1] = out1c / c;

            double d = out0d + out1d;
			_out_d[0] = out0d / d;
			_out_d[1] = out1d / d;
		}

        
        //leftover loop directly copied from belief2.h
        for (int j = end - (end%4); j < end; j++){
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
