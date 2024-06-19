
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

typedef struct {
	uint32_t capacity; // maximal size before a realocation will be needed
	uint32_t offset;   // offset to the next *potentially* empty cell
	uint32_t count;    // number of non-null elements in this set
	uint32_t size;     // size of the data section

	//
	// + - + - + - + - + - + - + - +
	// | A |   | C | D |   |   |   |
	// + - + - + - + - + - + - + - +
	//   0   1   2   3   4   5   6
	//
	// capacity = 7 ; there is enought memory to store 7 values
	// offset   = 1 ; data[1] is the first NULL element
	// count    = 3 ; there are three elements in total
	// size     = 4 ; data[3] is the last non-null value
	//
	// Please note that offset is *not* guarantieed to point to a
	// NULL value, but there are to be no NULL values *before* it.
	//

	void** data;
} IdVec;

/// Allocates new idvec and returns it, the array is initially empty and
/// has a capacity to hold 'initial' elements without realocation.
IdVec idvec_create(uint32_t initial);

/// Frees memory alloced for the idvec and any internally
/// used structures. The object is no longer usable after this call.
void idvec_free(IdVec* vec);

/// Removes a element at the given index from the array,
/// removing an element does not invalidate the index.
void idvec_remove(IdVec* vec, uint32_t index);

/// Adds a non-null element into the collection,
/// ordering of elements in the collection is unspecified.
void idvec_put(IdVec* vec, void* value);

/// helper macro for iterating idvec
#define IDVEC_FOREACH(Type, item, vector) \
	Type item; \
	for (int i = 0; i < vector.size; i ++) \
	if (item = vector.data[i])
