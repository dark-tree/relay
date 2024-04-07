
#include "sequence.h"

#define IDSEQ_SEED_MAX 200000

#if RAND_MAX < IDSEQ_SEED_MAX
#	error RAND_MAX must be equal or larger than IDSEQ_SEED_MAX
#endif

void idseq_begin(IdSequence* sequence, IdSeqMode mode) {

	sequence->mode = mode;

	if (mode == IDSEQ_RANDOMIZED) {
		int r1 = rand() % IDSEQ_SEED_MAX;
		int r2 = rand() % IDSEQ_SEED_MAX;
		int r3 = rand() % IDSEQ_SEED_MAX;

		sequence->v = r1;
		sequence->a = r2 * 4 + 1;
		sequence->c = r3 * 2 + 1;

		return;
	}

	if (mode == IDSEQ_MONOTONIC) {
		sequence->v = 1;

		return;
	}

}

uint32_t idseq_next(IdSequence* seq) {

	if (seq->mode == IDSEQ_RANDOMIZED) {
		return (seq->v = seq->a * seq->v + seq->c);
	}

	if (seq->mode == IDSEQ_MONOTONIC) {
		return seq->v ++;
	}

}
