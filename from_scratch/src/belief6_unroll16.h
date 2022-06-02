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

#ifndef NO_FABS
	__m256 abs_mask = _mm256_set1_ps(-0.0);
#endif
	__m256 eps = _mm256_set1_ps(EPS);
	__m256 one = _mm256_set1_ps(1.0);
	__m256 half = _mm256_set1_ps(0.5);

	size_t start;
	size_t end = off[0];
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

		size_t j = start;
		for (; j < max(0, (long long)end - 15); j+=16) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			__m256 valA1 = _mm256_set_ps(in_old[j+7], in_old[j+6], in_old[j+5], in_old[j+4], in_old[j+3], in_old[j+2], in_old[j+1], in_old[j+0]);
			__m256 valB1 = _mm256_set_ps(in_old[j+15], in_old[j+14], in_old[j+13], in_old[j+12], in_old[j+11], in_old[j+10], in_old[j+9], in_old[j+8]);
            __m256 valA0 = _mm256_sub_ps(one, valA1);
            __m256 valB0 = _mm256_sub_ps(one, valB1);

			__m256 outA0 = _mm256_add_ps(_mm256_div_ps(glob_00v, valA0), _mm256_div_ps(glob_10v, valA1));
			__m256 outA1 = _mm256_add_ps(_mm256_div_ps(glob_01v, valA0), _mm256_div_ps(glob_11v, valA1));
            __m256 outB0 = _mm256_add_ps(_mm256_div_ps(glob_00v, valA0), _mm256_div_ps(glob_10v, valB1));
			__m256 outB1 = _mm256_add_ps(_mm256_div_ps(glob_01v, valA0), _mm256_div_ps(glob_11v, valB1));

			__m256 out_sumA = _mm256_add_ps(outA0, outA1);
            __m256 out_sumB = _mm256_add_ps(outB0, outB1);

#ifdef DEBUG
			printf("unnorm: %f %f\n", outA0, outA1);
#endif

#ifdef NO_FABS
			__m256 lt_maskA = _mm256_cmp_ps(out_sumA, eps, _CMP_LT_OQ);
            __m256 lt_maskB = _mm256_cmp_ps(out_sumB, eps, _CMP_LT_OQ);
#else
			__m256 abs_out_sumA = _mm256_andnot_ps(abs_mask, out_sumA);
            __m256 abs_out_sumB = _mm256_andnot_ps(abs_mask, out_sumB);
			__m256 lt_maskA = _mm256_cmp_ps(abs_out_sumA, eps, _CMP_LT_OQ);
            __m256 lt_maskB = _mm256_cmp_ps(abs_out_sumB, eps, _CMP_LT_OQ);
#endif
			__m256 outA1_norm = _mm256_div_ps(outA1, out_sumA);
            __m256 outB1_norm = _mm256_div_ps(outB1, out_sumB);
			__m256 final_outA1 = _mm256_blendv_ps(outA1_norm, half, lt_maskA);
            __m256 final_outB1 = _mm256_blendv_ps(outB1_norm, half, lt_maskB);
			
			float_t *_outAa = (float_t *)(in + Gout[j]);
			float_t *_outAb = (float_t *)(in + Gout[j+1]);
			float_t *_outAc = (float_t *)(in + Gout[j+2]);
			float_t *_outAd = (float_t *)(in + Gout[j+3]);
			float_t *_outAe = (float_t *)(in + Gout[j+4]);
			float_t *_outAf = (float_t *)(in + Gout[j+5]);
			float_t *_outAg = (float_t *)(in + Gout[j+6]);
			float_t *_outAh = (float_t *)(in + Gout[j+7]);
            
            float_t *_outBa = (float_t *)(in + Gout[j+8]);
			float_t *_outBb = (float_t *)(in + Gout[j+9]);
			float_t *_outBc = (float_t *)(in + Gout[j+10]);
			float_t *_outBd = (float_t *)(in + Gout[j+11]);
			float_t *_outBe = (float_t *)(in + Gout[j+12]);
			float_t *_outBf = (float_t *)(in + Gout[j+13]);
			float_t *_outBg = (float_t *)(in + Gout[j+14]);
			float_t *_outBh = (float_t *)(in + Gout[j+15]);

			*_outAa = final_outA1[0];
			*_outAb = final_outA1[1];
			*_outAc = final_outA1[2];
			*_outAd = final_outA1[3];
			*_outAe = final_outA1[4];
			*_outAf = final_outA1[5];
			*_outAg = final_outA1[6];
			*_outAh = final_outA1[7];

            *_outBa = final_outB1[0];
			*_outBb = final_outB1[1];
			*_outBc = final_outB1[2];
			*_outBd = final_outB1[3];
			*_outBe = final_outB1[4];
			*_outBf = final_outB1[5];
			*_outBg = final_outB1[6];
			*_outBh = final_outB1[7];
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
#ifdef NO_FABS
			*_out = a < EPS ? .5 : out1 / a;
#else
			*_out = fabs(a) < EPS ? .5 : out1 / a;
#endif
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
		belief[i] = s < EPS ? .5 : b1/s;
	}
}

#endif
