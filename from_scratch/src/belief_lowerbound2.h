#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include <immintrin.h>
#define COMPACT_MESSAGE
#include "factor.h"

// Based on belief5.h and belief3_vector.h
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

	idx_t start;
	idx_t end = off[0];
	for (idx_t i = 0; i < n; i++) {
		start = end;
		end = off[i + 1];
		float_t pot_i1 = node_pot[i];
		float_t prod_tot0 = 1;

		for (auto k = start; k < end; k++)
			prod_tot0 += in_old[k];
		prod_tot0 *= pot_i1; // to force an use of pot_i1

		idx_t j = start;
		for (; j < max(0, end - 7); j+=8) {
			__m256 out0 = _mm256_set_ps(in_old[j+7], in_old[j+6], in_old[j+5], in_old[j+4], in_old[j+3], in_old[j+2], in_old[j+1], in_old[j+0]);

			float_t *_outa = (float_t *)(in + Gout[j]);
			float_t *_outb = (float_t *)(in + Gout[j+1]);
			float_t *_outc = (float_t *)(in + Gout[j+2]);
			float_t *_outd = (float_t *)(in + Gout[j+3]);
			float_t *_oute = (float_t *)(in + Gout[j+4]);
			float_t *_outf = (float_t *)(in + Gout[j+5]);
			float_t *_outg = (float_t *)(in + Gout[j+6]);
			float_t *_outh = (float_t *)(in + Gout[j+7]);
			*_outa = out0[0];
			*_outb = out0[1];
			*_outc = out0[2];
			*_outd = out0[3];
			*_oute = out0[4];
			*_outf = out0[5];
			*_outg = out0[6];
			*_outh = out0[7];
		}

		//leftover loop
		for (; j < end; j++) {
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

	for (int i = 0; i < n; i++) {
		float_t b0 = 1;
		float_t b1 = 1;
		for (int j = off[i]; j < off[i + 1]; j++){
			b0 *= 1-in[j];
			b1 *= in[j];
		}

		float_t s = b1 + b0;
		belief[i] = s < EPS ? .5 : b1/=s;
	}
}

#endif
