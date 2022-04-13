#ifndef BELIEF_TRANSLATOR_H
#define BELIEF_TRANSLATOR_H
#include <unistd.h>
#include "rating.h"
#include "util.h"

// Translates user and movie IDs into graph vertex IDs.
// Invariant: all u_lo < u_hi = m_lo < m_hi = max_out_id
// All user IDs get mapped to vertices \in [u_lo, u_hi - 1]
// All movie IDs get mapped to vertices \in [m_lo, m_hi - 1]

struct translator {
	size_t u_lo, u_hi;
	size_t m_lo, m_hi;
	size_t max_out_id;
	size_t _max_input_uid;
	size_t _max_input_mid;
};

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

#endif
