#ifndef BELIEF_LOAD_H
#define BELIEF_LOAD_H
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include "util.h"
#include "rating.h"

int from_file(const char *filename, rating_t **ratings)
{
	int capacity = 128; // initial capacity to save some initial reallocs

    FILE *fp = fopen(filename, "r");
    if (!fp)
        die("File %s cannot be opened.\n", filename);

	// skip first line
	while (fgetc(fp) != '\n' && !feof(fp))
		;
	
	*ratings = (rating_t *) malloc(capacity * sizeof **ratings);
	int i;
	for (i = 0; !feof(fp); i++) {
		if (i >= capacity) {
			capacity *= 2;
			*ratings = (rating_t *) realloc(*ratings, capacity * sizeof **ratings);
		}
		rating_t *ptr = &(*ratings)[i];
		if (fscanf(fp, "%d,%d,%f,%*d", &ptr->user, &ptr->movie, &ptr->rating) != 3)
			break;
	}

	*ratings = (rating_t *) realloc(*ratings, (i + 1) * sizeof **ratings);
	(*ratings)[i] = (rating_t){-1, -1, -1}; // terminator
	return i;
}
#endif
