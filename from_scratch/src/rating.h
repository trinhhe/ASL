#ifndef BELIEF_RATING_H
#define BELIEF_RATING_H
#include <assert.h>
#include <math.h>

/* Functions for working with the rating list, as parsed from a MovieLens CSV.
 * Throughout, we assume the ratings are grouped by user ID. */

typedef float float_t;

typedef struct {
	int user;
	int movie;
	float rating;
} rating_t;

/* Finds the first entry with a different UID */
rating_t *next_user(rating_t *E)
{
	int current_user = E->user;
	while (E->user > 0 && E->user == current_user)
		E++;
	if (E->user <= 0)
		return NULL;
	return E;
}

/* Finds the first entry with UID == given */
rating_t *find_user(rating_t *E, int uid)
{
	while (E && E->user != uid)
		E = next_user(E);
	return E;
}

/* Calculates the mean rating of an user. Assumes the ratings are grouped by
 * UID. */
float_t get_user_mean(rating_t *E)
{
	assert(E->user > 0);
	float_t sum = 0;
	int cnt = 0;
	for (rating_t *p = E; p->user == E->user; p++) {
		sum += p->rating;
		cnt++;
	}
	return sum / cnt;
}

/* Calculates the standard deviation of user's ratings. Assumes the ratings are
 * grouped by UID. */
float_t get_user_stddev(rating_t *E, float_t mean)
{
	assert(E->user > 0);
	float_t sum = 0;
	int cnt = 0;
	for (rating_t *p = E; p->user == E->user; p++) {
		float_t diff = p->rating - mean;
		sum += diff * diff;
		cnt++;
	}
	assert(cnt > 1);
	return sqrt(sum / cnt);
}

static float_t get_z_score(float_t x, float_t mean, float_t stddev)
{
	return (x - mean) / stddev;
}

#endif
