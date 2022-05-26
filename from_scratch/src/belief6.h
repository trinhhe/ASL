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

	__m256 abs_mask = _mm256_set1_ps(-0.0);
	__m256 eps = _mm256_set1_ps(EPS);
	__m256 one = _mm256_set1_ps(1.0);
	__m256 half = _mm256_set1_ps(0.5);

	idx_t start;
	idx_t end = off[0];
	for (idx_t i = 0; i < n; i++) {
		start = end;
		end = off[i + 1];
		float_t pot_i1 = node_pot[i];
		float_t pot_i0 = 1 - pot_i1;

		float_t prod_tot0 = 1;
		float_t prod_tot1 = 1;
		for (auto k = start; k < end; k++) {
			prod_tot0 *= 1-in_old[k];
			prod_tot1 *= in_old[k];
		}
		float_t glob00 = PROP_00 * pot_i0 * prod_tot0;
		float_t glob01 = PROP_01 * pot_i0 * prod_tot0;
		float_t glob10 = PROP_10 * pot_i1 * prod_tot1;
		float_t glob11 = PROP_11 * pot_i1 * prod_tot1;

		__m256 glob_00v = _mm256_set1_ps(glob00);
		__m256 glob_01v = _mm256_set1_ps(glob01);
		__m256 glob_10v = _mm256_set1_ps(glob10);
		__m256 glob_11v = _mm256_set1_ps(glob11);

		idx_t j = start;
		for (; j < max(0, end - 7); j+=8) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			__m256 val1 = _mm256_set_ps(in_old[j+7], in_old[j+6], in_old[j+5], in_old[j+4], in_old[j+3], in_old[j+2], in_old[j+1], in_old[j+0]);
			__m256 val0 = _mm256_sub_ps(one, val1);

			__m256 out0 = _mm256_add_ps(_mm256_div_ps(glob_00v, val0), _mm256_div_ps(glob_10v, val1));
			__m256 out1 = _mm256_add_ps(_mm256_div_ps(glob_01v, val0), _mm256_div_ps(glob_11v, val1));

			__m256 out_sum = _mm256_add_ps(out0, out1);

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif

			__m256 abs_out_sum = _mm256_andnot_ps(abs_mask, out_sum);
			__m256 lt_mask = _mm256_cmp_ps(abs_out_sum, eps, _CMP_LT_OQ);
			__m256 out1_norm = _mm256_div_ps(out1, out_sum);
			__m256 final_out1 = _mm256_blendv_ps(out1_norm, half, lt_mask);
			
			float_t *_outa = (float_t *)(in + Gout[j]);
			float_t *_outb = (float_t *)(in + Gout[j+1]);
			float_t *_outc = (float_t *)(in + Gout[j+2]);
			float_t *_outd = (float_t *)(in + Gout[j+3]);
			float_t *_oute = (float_t *)(in + Gout[j+4]);
			float_t *_outf = (float_t *)(in + Gout[j+5]);
			float_t *_outg = (float_t *)(in + Gout[j+6]);
			float_t *_outh = (float_t *)(in + Gout[j+7]);
			*_outa = final_out1[0];
			*_outb = final_out1[1];
			*_outc = final_out1[2];
			*_outd = final_out1[3];
			*_oute = final_out1[4];
			*_outf = final_out1[5];
			*_outg = final_out1[6];
			*_outh = final_out1[7];
		}

#ifndef GRAPH_PADDING
		//leftover loop
        for (; j < end; j++) {
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
			*_out = fabs(a) < EPS ? .5 : out1 / a;
		}
#endif
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
