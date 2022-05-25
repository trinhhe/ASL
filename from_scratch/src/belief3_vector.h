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

		//unrolled just like belief_simpleUnroll.h
        size_t end = off[i + 1];
#ifdef VEC2
		__m256 glob_eq = _mm256_set_ps(glob11, glob00, glob11, glob00, glob11, glob00, glob11, glob00);
		__m256 glob_diff = _mm256_set_ps(glob01, glob10, glob01, glob10, glob01, glob10, glob01, glob10);
        __m256i rev_idx = _mm256_set_epi32(0x2, 0x3, 0x0, 0x1, 0x2, 0x3, 0x0, 0x1);
		__m256 abs_mask = _mm256_set1_ps(-0.0);
		__m256 eps = _mm256_set1_ps(1e-6);
		__m256 half = _mm256_set1_ps(0.5);

		// TODO: Are these correct?
		__m256i msg1_mask = _mm256_set_epi64x(0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF);
		__m256i msg2_mask = _mm256_set_epi64x(0x0000000000000000, 0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000);
		__m256i msg3_mask = _mm256_set_epi64x(0x0000000000000000, 0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000);
		__m256i msg4_mask = _mm256_set_epi64x(0xFFFFFFFFFFFFFFFF, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000);


		// RH: I think the intermediate loop bound should be end & ~3 (as opposed to end - 3), otherwise, we may count something twice in the leftover loop.
        for (int j = off[i]; j < end - 3; j+=4) {
#ifdef GRAPH_PADDING
			__m256 val = _mm256_load_ps((const float*)&(in_old[j]));
#else
			__m256 val = _mm256_loadu_ps((const float*)&(in_old[j]));
#endif
			
			__m256 val_rev = _mm256_permutevar_ps(val, rev_idx);

			__m256 out = _mm256_add_ps(_mm256_div_ps(glob_eq, val), _mm256_div_ps(glob_diff, val_rev));

			__m256 out_rev = _mm256_permutevar_ps(out, rev_idx);
			__m256 out_sum = _mm256_add_ps(out, out_rev);

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif

			__m256 abs_out_sum = _mm256_andnot_ps(abs_mask, out_sum);
			__m256 lt_mask = _mm256_cmp_ps(abs_out_sum, eps, _CMP_LT_OQ);
			__m256 out_norm = _mm256_div_ps(out, out_sum); 
			__m256 final_out = _mm256_blendv_ps(out_norm, half, lt_mask);

			float_t *_outa = (float_t *)(in + Gout[j]);
			float_t *_outb = (float_t *)(in + Gout[j+1]);
			float_t *_outc = (float_t *)(in + Gout[j+2]);
			float_t *_outd = (float_t *)(in + Gout[j+3]);
#ifdef GRAPH_PADDING
			_mm256_maskstore_ps(_outa, msg1_mask, final_out);
			_mm256_maskstore_ps(_outb, msg2_mask, final_out);
			_mm256_maskstore_ps(_outc, msg3_mask, final_out);
			_mm256_maskstore_ps(_outd, msg4_mask, final_out);
#else
			_outa[0] = final_out[0];
			_outa[1] = final_out[1];
			_outb[0] = final_out[2];
			_outb[1] = final_out[3];
			_outc[0] = final_out[4];
			_outc[1] = final_out[5];
			_outd[0] = final_out[6];
			_outd[1] = final_out[7];
#endif
		}

		// RH: I think this leftover loop can be left out if GRAPH_PADDING. But it shouldn't matter if we fix the loop bounds first (see above)
		//leftover loop
        for (int j = max(0, end-3); j < end; j++) {
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
#else
		__m256 glob_00v = _mm256_set1_ps(glob00);
		__m256 glob_01v = _mm256_set1_ps(glob01);
		__m256 glob_10v = _mm256_set1_ps(glob10);
		__m256 glob_11v = _mm256_set1_ps(glob11);
		__m256 abs_mask = _mm256_set1_ps(-0.0);
		__m256 eps = _mm256_set1_ps(1e-6);
		__m256 half = _mm256_set1_ps(0.5);

		// RH: same here, I think it should be end & ~7
        for (int j = off[i]; j < end - 7; j+=8) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif
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

			__m256 out0 = _mm256_add_ps(_mm256_div_ps(glob_00v, val0), _mm256_div_ps(glob_10v, val1));
			__m256 out1 = _mm256_add_ps(_mm256_div_ps(glob_01v, val0), _mm256_div_ps(glob_11v, val1));

			__m256 out_sum = _mm256_add_ps(out0, out1);

#ifdef DEBUG
			printf("unnorm: %f %f\n", out0, out1);
#endif

			__m256 abs_out_sum = _mm256_andnot_ps(abs_mask, out_sum);
			__m256 lt_mask = _mm256_cmp_ps(abs_out_sum, eps, _CMP_LT_OQ);
			__m256 out0_norm = _mm256_div_ps(out0, out_sum);
			__m256 out1_norm = _mm256_div_ps(out1, out_sum);
			__m256 final_out0 = _mm256_blendv_ps(out0_norm, half, lt_mask);
			__m256 final_out1 = _mm256_blendv_ps(out1_norm, half, lt_mask);

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
# endif

	}

}

#include "get_belief_stock.h"
#endif
