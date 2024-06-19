
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

#pragma once
#include "external.h"

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
