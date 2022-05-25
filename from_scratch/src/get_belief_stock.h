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
