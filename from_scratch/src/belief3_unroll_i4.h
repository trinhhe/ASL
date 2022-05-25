#ifndef BELIEF_BELIEF_H
#define BELIEF_BELIEF_H
#include <string.h>
#include "factor.h"

/* Do one step of belief propagation. */
//based on belief3.h
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

	int start, start2, start3, start4;
	int end = off[0];
    int end2, end3, end4;
    int i;
	for (i = 0; i < n-3; i+=4) {
		float_t pot_ia0 = ((float_t *)&node_pot[i])[0];
		float_t pot_ia1 = ((float_t *)&node_pot[i])[1];
        float_t pot_ib0 = ((float_t *)&node_pot[i+1])[0];
        float_t pot_ib1 = ((float_t *)&node_pot[i+1])[1];
        float_t pot_ic0 = ((float_t *)&node_pot[i+2])[0];
        float_t pot_ic1 = ((float_t *)&node_pot[i+2])[1];
        float_t pot_id0 = ((float_t *)&node_pot[i+3])[0];
        float_t pot_id1 = ((float_t *)&node_pot[i+3])[1];

		float_t prod_tota0 = 1;
		float_t prod_tota1 = 1;
        float_t prod_totb0 = 1;
        float_t prod_totb1 = 1;
        float_t prod_totc0 = 1;
		float_t prod_totc1 = 1;
        float_t prod_totd0 = 1;
        float_t prod_totd1 = 1;

		start = end;
		end = off[i + 1];
        start2 = end;
        end2 = off[i + 2];
        start3 = end2;
        end3 = off[i + 3];
        start4 = end3;
        end4 = off[i + 4];
		for (int k = start; k < end; k++) {
			prod_tota0 *= ((float_t *)&in_old[k])[0];
			prod_tota1 *= ((float_t *)&in_old[k])[1];
		}

        for (int k = start2; k < end2; k++) {
			prod_totb0 *= ((float_t *)&in_old[k])[0];
			prod_totb1 *= ((float_t *)&in_old[k])[1];
		}

        for (int k = start3; k < end3; k++) {
			prod_totc0 *= ((float_t *)&in_old[k])[0];
			prod_totc1 *= ((float_t *)&in_old[k])[1];
		}

        for (int k = start4; k < end4; k++) {
			prod_totd0 *= ((float_t *)&in_old[k])[0];
			prod_totd1 *= ((float_t *)&in_old[k])[1];
		}

		float_t globa00 = PROP_00 * pot_ia0 * prod_tota0;
		float_t globa01 = PROP_01 * pot_ia0 * prod_tota0;
		float_t globa10 = PROP_10 * pot_ia1 * prod_tota1;
		float_t globa11 = PROP_11 * pot_ia1 * prod_tota1;

        float_t globb00 = PROP_00 * pot_ib0 * prod_totb0;
		float_t globb01 = PROP_01 * pot_ib0 * prod_totb0;
		float_t globb10 = PROP_10 * pot_ib1 * prod_totb1;
		float_t globb11 = PROP_11 * pot_ib1 * prod_totb1;

        float_t globc00 = PROP_00 * pot_ic0 * prod_totc0;
		float_t globc01 = PROP_01 * pot_ic0 * prod_totc0;
		float_t globc10 = PROP_10 * pot_ic1 * prod_totc1;
		float_t globc11 = PROP_11 * pot_ic1 * prod_totc1;

        float_t globd00 = PROP_00 * pot_id0 * prod_totd0;
		float_t globd01 = PROP_01 * pot_id0 * prod_totd0;
		float_t globd10 = PROP_10 * pot_id1 * prod_totd1;
		float_t globd11 = PROP_11 * pot_id1 * prod_totd1;

		for (int j = start; j < end; j++) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0 = ((float_t *)&in_old[j])[0];
			float_t val1 = ((float_t *)&in_old[j])[1];
			float_t *_out = (float_t *)(in + Gout[j]);
			float_t out0 = 0;
			float_t out1 = 0;

			out0 += globa00 / val0;
			out1 += globa01 / val0;
			out0 += globa10 / val1;
			out1 += globa11 / val1;

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

        for (int j = start2; j < end2; j++) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0 = ((float_t *)&in_old[j])[0];
			float_t val1 = ((float_t *)&in_old[j])[1];
			float_t *_out = (float_t *)(in + Gout[j]);
			float_t out0 = 0;
			float_t out1 = 0;

			out0 += globa00 / val0;
			out1 += globa01 / val0;
			out0 += globa10 / val1;
			out1 += globa11 / val1;

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

        for (int j = start3; j < end3; j++) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0 = ((float_t *)&in_old[j])[0];
			float_t val1 = ((float_t *)&in_old[j])[1];
			float_t *_out = (float_t *)(in + Gout[j]);
			float_t out0 = 0;
			float_t out1 = 0;

			out0 += globa00 / val0;
			out1 += globa01 / val0;
			out0 += globa10 / val1;
			out1 += globa11 / val1;

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

        for (int j = start4; j < end4; j++) {
#ifdef GRAPH_PADDING
			if (Gout[j] == -1)
				break; // reached padding
#endif

			float_t val0 = ((float_t *)&in_old[j])[0];
			float_t val1 = ((float_t *)&in_old[j])[1];
			float_t *_out = (float_t *)(in + Gout[j]);
			float_t out0 = 0;
			float_t out1 = 0;

			out0 += globa00 / val0;
			out1 += globa01 / val0;
			out0 += globa10 / val1;
			out1 += globa11 / val1;

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

    //leftover loop TODO REPLACE RIGHT ONE
	for (; i < n; i++) {
		float_t pot_i0 = ((float_t *)&node_pot[i])[0];
		float_t pot_i1 = ((float_t *)&node_pot[i])[1];

		float_t prod_tot0 = 1;
		float_t prod_tot1 = 1;

		start = off[i];
		end = off[i + 1];
		for (int k = start; k < end; k++) {
			prod_tot0 *= ((float_t *)&in_old[k])[0];
			prod_tot1 *= ((float_t *)&in_old[k])[1];
		}

		float_t glob00 = PROP_00 * pot_i0 * prod_tot0;
		float_t glob01 = PROP_01 * pot_i0 * prod_tot0;
		float_t glob10 = PROP_10 * pot_i1 * prod_tot1;
		float_t glob11 = PROP_11 * pot_i1 * prod_tot1;

		for (int j = start; j < end; j++) {
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
