#ifndef BELIEF_FACTOR_H
#define BELIEF_FACTOR_H

#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "constants.h"
#include "rating.h"
#include "util.h"

typedef struct {
	float L;
	float D;
} statevector_t;

typedef statevector_t msg_t;
typedef statevector_t potential_t;
// Semantically these two things are different, but we can treat them the same

struct translator {
	size_t u_lo, u_hi;
	size_t m_lo, m_hi;
	size_t max_out_id;
	size_t _max_input_uid;
	size_t _max_input_mid;
};

typedef struct {
	size_t n; // Total number of vertices, size of off, node_pot
	size_t m; // Total number of half-edges (i.e., 2|E|), size of in, in_old, out
	size_t *off; // off[v] = the offset where the v-th vertex begins
	msg_t *in; // buffer for incoming messages, the ones for vertex v are at in[off[v]] … in[off[v + 1] - 1]
	msg_t *in_old; // the same, but for the previous iteration, these two get swapped after every iteration
	size_t *out; // out[off[v]] … out[off[v + 1] - 1] contain the indices in in where the messages from v should be delivered
	potential_t *node_pot;
	struct translator tr;
	// debug info:
	size_t *eix; // the index of the edge in the input sequence
	rating_t *E; // the original input sequence of edges
} graph_t;

// TODO: this works quite well for dense IDs, but is inefficient for sparse IDs
void translator_init(struct translator *trans, rating_t *E) {
	*trans = (struct translator){0};
	for (; E->user != -1; E++) {
		trans->_max_input_uid = max(trans->_max_input_uid, E->user - 1);
		trans->_max_input_mid = max(trans->_max_input_mid, E->movie - 1);
	}
	trans->_max_input_mid++;
	trans->_max_input_uid++;
	trans->max_out_id = trans->_max_input_uid + trans->_max_input_mid;
	trans->u_lo = 0;
	trans->u_hi = trans->m_lo = trans->_max_input_uid;
	trans->m_hi = trans->m_lo + trans->_max_input_mid;
}

int translator_user_to_id(struct translator *trans, int user) {
	return user - 1;
}

int translator_movie_to_id(struct translator *trans, int movie) {
	return movie - 1 + trans->_max_input_uid;
}

int translator_id_to_usermovie(struct translator *trans, int id, bool *movie) {
	if (id >= trans->m_lo) {
		id -= trans->m_lo;
		*movie = true;
	} else {
		*movie = false;
	}

	return id + 1;
}

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
			p->rating = -1; // marks a nonexistent edge
	}
}

void graph_from_edge_list(rating_t *E, int target_uid, graph_t *_G)
{
	graph_t G = {0};
	translator_init(&G.tr, E);
	G.n = G.tr.max_out_id;
	G.off = (size_t *) calloc(G.n + 1, sizeof *G.off);

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

	// degrees => offsets
	int tmp = 0;
	for (int i = 0; i <= G.n; i++) {
		int tmp2 = G.off[i];
		G.off[i] = tmp;
		tmp += tmp2;
	}

	/*
	for (int i = 0; i <= G.n; i++) {
		printf("%ld ", G.off[i]);
	}
	printf("\n"); */

	G.m = G.off[G.n]; // G.off[n] serves as a terminator

	G.out = (size_t *) malloc(G.m * sizeof *G.out);
	G.eix = (size_t *) malloc(G.m * sizeof *G.eix);
	G.in = (msg_t *) calloc(G.m, sizeof *G.in);
	G.in_old = (msg_t *) calloc(G.m, sizeof *G.in_old);

	// second pass to actually work with the edges
	for (rating_t *p = E; p->user > 0; p++) {
		if (p->rating < 0)
			continue;
		int u = translator_user_to_id(&G.tr, p->user);
		int v = translator_movie_to_id(&G.tr, p->movie);
		G.out[G.off[u]] = G.off[v];
		G.out[G.off[v]] = G.off[u];
		G.eix[G.off[u]] = G.eix[G.off[v]] = p - E;
		G.off[u]++;
		G.off[v]++;
	}

	// now we need to "unwind" off:
	for (int i = G.n - 1; i >= 0; i--)
		G.off[i + 1] = G.off[i];
	G.off[0] = 0;

	// finally, do target-user-dependent calculations
	rating_t *target = find_user(E, target_uid);
	if (!target)
		die("User with UID %d does not exist\n", target_uid);
	assert(target->user == target_uid);
	float_t target_mean = get_user_mean(target);
	float_t target_stddev = get_user_stddev(target, target_mean);
	G.node_pot = (potential_t *) calloc(G.n, sizeof *G.node_pot);
	for (int i = 0; i < G.n; i++)
		G.node_pot[i] = (potential_t){.5, .5};

	for (rating_t *p = target; p->user == target_uid; p++) {
		int v = translator_movie_to_id(&G.tr, p->movie);
		float_t raw_Lpot = .5 + get_z_score(p->rating, target_mean, target_stddev) / NORMALISE_P;
		G.node_pot[v].L = fmax(0.1, fmin(0.9, raw_Lpot));
		G.node_pot[v].D = 1 -  G.node_pot[v].L;
	}

	G.E = E;
	*_G = G;
}

void dump_graph(graph_t *G) {
	printf("Graph, n = %zd (%zd user IDs + %zd movie IDs), m = %zd\n", G->n, G->tr.u_hi - G->tr.u_lo, G->tr.m_hi - G->tr.m_lo, G->m);
	printf("offsets: ");
	for (int i = 0; i <= G->n; i++)
		printf("%zd ", G->off[i]);
	printf("\n");
	printf("edges:\n");
	for (int v = 0; v < G->n; v++) {
		printf("[ %d:  ", v);
		for (int i = G->off[v]; i < G->off[v + 1]; i++) {
			size_t e = G->eix[i];
			printf("%zd (u%d->m%d, r=%.0f) ", e, G->E[e].user, G->E[e].movie, G->E[e].rating);
		}
		printf("]\n");
	}
	printf("\n");
}

#endif
