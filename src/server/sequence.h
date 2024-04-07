
#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef enum {
	IDSEQ_RANDOMIZED,
	IDSEQ_MONOTONIC
} IdSeqMode;

typedef struct {
	IdSeqMode mode;
	uint32_t a, c, v;
} IdSequence;

/// Create a new non-repeating integer sequence,
/// where mode controls the ordering of the seqence elements.
void idseq_begin(IdSequence* seq, IdSeqMode mode);

/// Fetch the next unique integer from the
/// non-repeating sequence.
uint32_t idseq_next(IdSequence* seq);
