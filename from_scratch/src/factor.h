#ifndef BELIEF_FACTOR_H
#define BELIEF_FACTOR_H

#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "constants.h"
#include "rating.h"
#include "translator.h"
#include "util.h"

/* Contains the definition of a factor graph and functions for its construction
 * from the list of ratings. */

#ifdef COMPACT_MESSAGE
typedef u_int32_t idx_t;
typedef float_t msg_t;
typedef float_t potential_t;
typedef float_t statevector_t;
#else
typedef struct {
	float L;
	float D;
} statevector_t;

typedef size_t idx_t;
typedef statevector_t msg_t;
typedef statevector_t potential_t;

void normalise_msg(msg_t *m) {
	// TODO: numerical stability
	float_t s = m->L + m->D;
	if (s < EPS) {
		m->L = m->D = .5;
	} else {
		m->L /= s;
		m->D /= s;
	}
}
#endif

typedef struct {
	idx_t n; // Total number of vertices
	idx_t m; // Total number of half-edges (i.e., 2|E|)
	idx_t *off; // size = n; off[v] = the offset where the v-th vertex begins
	msg_t *in; // size = m; buffer for incoming messages, the ones for vertex v are at in[off[v]] … in[off[v + 1] - 1]
	msg_t *in_old; // size = m; the same, but for the previous iteration, these two get swapped after every iteration
	idx_t *out; // size = m; out[off[v]] … out[off[v + 1] - 1] contain the indices in in where the messages from v should be delivered
	potential_t *node_pot; // size = n; the potential for each node
	struct translator tr; // translates user and movie ids into graph vertex ids
	statevector_t *belief; // the final belief of each node
	// debug info:
	rating_t *E; // the original input sequence of edges
	idx_t *eix; // the index of the edge in the input sequence
	int target_uid; // the id of the target vertex
} graph_t;

static constexpr int GRAPH_IN_ALIGN_NMEMB = SIMD_ALIGN_BYTES / sizeof *graph_t::in;


/* For each user, mark the movies that are below the user's average, with a rating of -1. */
void filter_below_mean(rating_t *E)
{
	int prev_user = -1;
	int threshold = -1;
	for (rating_t *p = E; p->user > 0; p++) {
		if (p->user != prev_user) { // new user, new threshold
			threshold = get_user_mean(p);
			prev_user = p->user;
		}

		if (p->rating < threshold)
			p->rating = -1;
	}
}

/* Constructs a factor graph from a list of ratings, as per the BP paper. */
void graph_from_edge_list(rating_t *E, int target_uid, graph_t *_G)
{
	graph_t G = {0};
	translator_init(&G.tr, E);
	G.n = G.tr.max_out_id;
	G.off = (idx_t *) aligned_calloc(G.n + 1, sizeof *G.off);
	G.belief = (statevector_t *) aligned_calloc(G.n, sizeof *G.belief);

	// make all ``disabled'' edges have rating = -1
	filter_below_mean(E);

	// first pass to calculate degrees into G.off
	for (rating_t *p = E; p->user > 0; p++) {
		if (p->rating < 0)
			continue;
		int u = translator_user_to_id(&G.tr, p->user);
		int v = translator_movie_to_id(&G.tr, p->movie);
		G.off[u]++;
		G.off[v]++;
	}

#ifdef GRAPH_PADDING
	for (idx_t i = 0; i < G.n; i++) {
		G.off[i] = round_up(G.off[i], GRAPH_IN_ALIGN_NMEMB);
	}
#endif

	// degrees => offsets, using prefix sums
	int tmp = 0;
	for (idx_t i = 0; i <= G.n; i++) { // yes, <= G.n, as G.off[n] serves as a terminator
		int tmp2 = G.off[i];
		G.off[i] = tmp;
		tmp += tmp2;
	}

	G.m = G.off[G.n];

	G.out = (idx_t *) aligned_malloc(G.m * sizeof *G.out);
	G.eix = (idx_t *) aligned_malloc(G.m * sizeof *G.eix);
	G.in = (msg_t *) aligned_malloc((G.m + GRAPH_IN_ALIGN_NMEMB) * sizeof *G.in);
	G.in_old = (msg_t *) aligned_malloc((G.m + GRAPH_IN_ALIGN_NMEMB) * sizeof *G.in_old);
	// introduce some space before, to allow writing at position -1
	G.in += GRAPH_IN_ALIGN_NMEMB;
	G.in_old += GRAPH_IN_ALIGN_NMEMB;

	// "zero"-initialise everything, will get rewritten below everywhere except
	// for padding where this zeroing out is actually important
	for (idx_t i = 0; i < G.m; i++) {
#ifdef COMPACT_MESSAGE
		G.in_old[i] = G.in[i] = .5;
#else
		G.in_old[i] = G.in[i] = (msg_t){1, 1};
#endif
		G.out[i] = -1;
		G.eix[i] = -1;
	}

	// second pass to actually work with the edges
	for (rating_t *p = E; p->user > 0; p++) {
		if (p->rating < 0)
			continue;
		int u = translator_user_to_id(&G.tr, p->user);
		int v = translator_movie_to_id(&G.tr, p->movie);

		idx_t off_v = G.off[v];
		idx_t off_u = G.off[u];
		// construct the pointer to the ``other end'' of this edge
		G.out[off_u] = off_v;
		G.out[off_v] = off_u;
#ifdef COMPACT_MESSAGE
		G.in[off_u] = G.in_old[off_u] = .5;
		G.in[off_v] = G.in_old[off_v] = .5;
#else
		G.in[off_u] = G.in_old[off_u] = (msg_t){.5, .5};
		G.in[off_v] = G.in_old[off_v] = (msg_t){.5, .5};
#endif

		G.eix[G.off[u]] = G.eix[G.off[v]] = p - E;
		G.off[u]++;
		G.off[v]++;
	}

	// now we need to "unwind" off:
	for (rating_t *p = E; p->user > 0; p++) {
		if (p->rating < 0)
			continue;
		int u = translator_user_to_id(&G.tr, p->user);
		int v = translator_movie_to_id(&G.tr, p->movie);
		G.off[u]--;
		G.off[v]--;
	}

	// finally, do target-user-dependent calculations
	rating_t *target = find_user(E, target_uid);
	if (!target)
		die("User with UID %d does not exist\n", target_uid);
	assert(target->user == target_uid);
	float_t target_mean = get_user_mean(target);
	float_t target_stddev = get_user_stddev(target, target_mean);
	G.node_pot = (potential_t *) aligned_calloc(G.n, sizeof *G.node_pot);
	for (idx_t i = 0; i < G.n; i++){
#ifdef COMPACT_MESSAGE
		G.node_pot[i] = .5; // defaults
#else
		G.node_pot[i] = (potential_t){.5, .5}; // defaults
#endif
	}

	for (rating_t *p = target; p->user == target_uid; p++) {
		int v = translator_movie_to_id(&G.tr, p->movie);
		float_t raw_Lpot = .5 + get_z_score(p->rating, target_mean, target_stddev) / NORMALISE_P;
#ifdef COMPACT_MESSAGE
		G.node_pot[v] = fmax(0.1, fmin(0.9, raw_Lpot));
#else
		G.node_pot[v].L = fmax(0.1, fmin(0.9, raw_Lpot));
		G.node_pot[v].D = 1 -  G.node_pot[v].L;
#endif
	}

	G.E = E;
	*_G = G;
}

void graph_destroy(graph_t *G) {
	// "remember" about the scratch space right before the pointers we introduced previously
	G->in -= GRAPH_IN_ALIGN_NMEMB;
	G->in_old -= GRAPH_IN_ALIGN_NMEMB;
	free(G->off);
	free(G->in);
	free(G->in_old);
	free(G->out);
	free(G->node_pot);
	free(G->belief);
	free(G->eix);
}

void dump_beliefs(graph_t *G) {
	for (idx_t v = G->tr.m_lo; v < G->tr.m_hi; v++){
#ifdef COMPACT_MESSAGE
		printf("%.3f ", G->belief[v]);
#else
		printf("%.3f ", G->belief[v].L);
#endif
	}
	printf("\n");
}

void dump_msgs(graph_t *G) {
	for (idx_t i = 0; i < G->n; i++) {
		for (idx_t j = G->off[i]; j < G->off[i + 1]; j++) {
#ifdef COMPACT_MESSAGE
			printf("%.3f ", G->in[j]);
#else
			printf("%.3f ", G->in[j].L);
#endif
		}
		printf("\n");
	}
}

void dump_graph(graph_t *G) {
#ifdef COMPACT_MESSAGE
	printf("Graph, n = %d (%zd user IDs + %zd movie IDs), m = %d\n", G->n, G->tr.u_hi - G->tr.u_lo, G->tr.m_hi - G->tr.m_lo, G->m);
#else
	printf("Graph, n = %zd (%zd user IDs + %zd movie IDs), m = %zd\n", G->n, G->tr.u_hi - G->tr.u_lo, G->tr.m_hi - G->tr.m_lo, G->m);
#endif
	printf("offsets: ");
	for (idx_t i = 0; i <= G->n; i++){
#ifdef COMPACT_MESSAGE
		printf("%d ", G->off[i]);
#else
		printf("%zd ", G->off[i]);
#endif
	}
	printf("\n");
	printf("edges:\n");
	for (idx_t v = 0; v < G->n; v++) {
		printf("[ %d:  ", (int)v);
		for (idx_t i = G->off[v]; i < G->off[v + 1]; i++) {
			if (G->eix[i] == -1)
				break;
			idx_t e = G->eix[i];
#ifdef VERBOSE_DUMP
#ifdef COMPACT_MESSAGE
			printf("%d (u%d->m%d, r=%.0f, out=%d) ", e, G->E[e].user, G->E[e].movie, G->E[e].rating, G->out[i]);
#else
			printf("%zd (u%d->m%d, r=%.0f, out=%zd) ", e, G->E[e].user, G->E[e].movie, G->E[e].rating, G->out[i]);
#endif
#else
#ifdef COMPACT_MESSAGE
			printf("%d (u%d->m%d, r=%.0f) ", e, G->E[e].user, G->E[e].movie, G->E[e].rating);
#else
			printf("%zd (u%d->m%d, r=%.0f) ", e, G->E[e].user, G->E[e].movie, G->E[e].rating);
#endif
#endif
		}
		printf("]\n");
	}
	printf("\n");
	printf("node potentials:\n");
	for (idx_t v = 0; v < G->n; v++) {
#ifdef COMPACT_MESSAGE
		printf("[%.3f %.3f] ", G->node_pot[v], 1-G->node_pot[v]);
#else
		printf("[%.3f %.3f] ", G->node_pot[v].L, G->node_pot[v].D);
#endif
	}
	printf("\n");
	printf("beliefs (= predicted film ratings):\n");
	dump_beliefs(G);
}

#endif
