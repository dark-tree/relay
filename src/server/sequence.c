
/*
 *  Copyright (C) 2024 magistermaks
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
