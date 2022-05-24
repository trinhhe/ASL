#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include <immintrin.h>
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
		for (int k = off[i]; k < off[i + 1]; k++) {
			prod_tot0 *= ((float_t *)&in_old[k])[0];
			prod_tot1 *= ((float_t *)&in_old[k])[1];
		}
		float_t glob00 = PROP_00 * pot_i0 * prod_tot0;
		float_t glob01 = PROP_01 * pot_i0 * prod_tot0;
		float_t glob10 = PROP_10 * pot_i1 * prod_tot1;
		float_t glob11 = PROP_11 * pot_i1 * prod_tot1;

        float_t glob[4];
        float_t PROP[4] = {PROP_00, PROP_01, PROP_10, PROP_11};
        float_t pot[4] = {pot_i0, pot_i0, pot_i1, pot_i1};
        float_t prod_tot[4] = {prod_tot0, prod_tot0, prod_tot1, prod_tot1};

		__m256 glob_00v = _mm256_set1_ps(glob00);
		__m256 glob_01v = _mm256_set1_ps(glob01);
		__m256 glob_10v = _mm256_set1_ps(glob10);
		__m256 glob_11v = _mm256_set1_ps(glob11);
		__m256 abs_mask = _mm256_set1_ps(-0.0);
		__m256 eps = _mm256_set1_ps(1e-6);
		__m256 half = _mm256_set1_ps(0.5);

		//unrolled just like belief_simpleUnroll.h
        size_t end = off[i + 1];
        for (int j = off[i]; j < end - 7; j+=8) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			__m256 out0 = _mm256_setzero_ps();
			__m256 out1 = _mm256_setzero_ps();

			__m256 val0 = _mm256_set_ps(((float_t *)&in_old[j+7])[0],
			                             ((float_t *)&in_old[j+6])[0],
										 ((float_t *)&in_old[j+5])[0],
										 ((float_t *)&in_old[j+4])[0],
										 ((float_t *)&in_old[j+3])[0],
										 ((float_t *)&in_old[j+2])[0],
										 ((float_t *)&in_old[j+1])[0],
										 ((float_t *)&in_old[j+0])[0]);
			__m256 val1 = _mm256_set_ps(((float_t *)&in_old[j+7])[1],
			                             ((float_t *)&in_old[j+6])[1],
										 ((float_t *)&in_old[j+5])[1],
										 ((float_t *)&in_old[j+4])[1],
										 ((float_t *)&in_old[j+3])[1],
										 ((float_t *)&in_old[j+2])[1],
										 ((float_t *)&in_old[j+1])[1],
										 ((float_t *)&in_old[j+0])[1]);

			out0 = _mm256_add_ps(_mm256_div_ps(glob_00v, val0), _mm256_div_ps(glob_10v, val1));
			out1 = _mm256_add_ps(_mm256_div_ps(glob_01v, val0), _mm256_div_ps(glob_11v, val1));

			__m256 out_sum = _mm256_add_ps(out0, out1);

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif

			__m256 abs_out_sum = _mm256_andnot_ps(abs_mask, out_sum);
			__m256 lt_mask = _mm256_cmp_ps(abs_out_sum, eps, _CMP_LT_OQ);
			__m256 out0_norm = _mm256_div_ps(out0, out_sum); // Is _mm256_mask_div_ps only for avx-512?
			__m256 out1_norm = _mm256_div_ps(out1, out_sum); // Is _mm256_mask_div_ps only for avx-512?
			__m256 final_out0 = _mm256_blendv_ps(out0_norm, half, lt_mask);
			__m256 final_out1 = _mm256_blendv_ps(out1_norm, half, lt_mask);

			// Is _mm256_mask_storeu_ps supported on SkyLake?
			float_t *_outa = (float_t *)(in + Gout[j]);
			float_t *_outb = (float_t *)(in + Gout[j+1]);
			float_t *_outc = (float_t *)(in + Gout[j+2]);
			float_t *_outd = (float_t *)(in + Gout[j+3]);
			float_t *_oute = (float_t *)(in + Gout[j+4]);
			float_t *_outf = (float_t *)(in + Gout[j+5]);
			float_t *_outg = (float_t *)(in + Gout[j+6]);
			float_t *_outh = (float_t *)(in + Gout[j+7]);
			_outa[0] = final_out0[0];
			_outa[1] = final_out1[0];
			_outb[0] = final_out0[1];
			_outb[1] = final_out1[1];
			_outc[0] = final_out0[2];
			_outc[1] = final_out1[2];
			_outd[0] = final_out0[3];
			_outd[1] = final_out1[3];
			_oute[0] = final_out0[4];
			_oute[1] = final_out1[4];
			_outf[0] = final_out0[5];
			_outf[1] = final_out1[5];
			_outg[0] = final_out0[6];
			_outg[1] = final_out1[6];
			_outh[0] = final_out0[7];
			_outh[1] = final_out1[7];
		}

        //leftover loop
        for (int j = max(0, end-7); j < end; j++) {
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
			if (fabs(a) < 1e-6) {
				_out[0] = .5;
				_out[1] = .5;
			} else {
				_out[0] = out0 / a;
				_out[1] = out1 / a;
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

