
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

#include "vec.h"

IdVec idvec_create(uint32_t initial) {
	IdVec vec;

	vec.capacity = initial;
	vec.offset = 0;
	vec.count = 0;
	vec.size = 0;

	vec.data = (void**) calloc(initial, sizeof(void*));

	return vec;
}

void idvec_free(IdVec* this) {

	this->capacity = 0;
	this->offset = 0;
	this->count = 0;
	this->size = 0;

	free(this->data);
	this->data = NULL;

}

void idvec_remove(IdVec* this, uint32_t index) {

	if (this->data[index] == NULL) {
		return;
	}

	this->data[index] = NULL;
	this->count --;

	// make sure no space can be lost
	if (this->offset > index) {
		this->offset = index;
	}

	// let's not waste time if the last element was removed
	if (this->size == index + 1) {

		// auto trim size so that is corectly
		// reflects the actual position of the
		// last non-null value
		do {
			this->size --;

			if (this->size == 0) {
				break;
			}
		} while (this->data[this->size - 1] == NULL);

	}
}

void idvec_put(IdVec* this, void* value) {

	// this code leaves offset pointing at one before the empty element
	// but that is also fine and allows the code to be simpler
	while (true) {

		// check the value pointed to by offset
		if (this->data[this->offset] == NULL) {
			this->data[this->offset] = value;
			break;
		}

		// go to the next element and potentially realocate
		if ((++ this->offset) >= this->capacity) {
			const uint32_t new_cap = this->capacity * 2;

			const uint64_t old_size = this->capacity * sizeof(void*);
			const uint64_t new_size = new_cap * sizeof(void*);
			this->data = realloc(this->data, new_size);

			memset(this->data + this->capacity, 0, new_size - old_size);
			this->capacity = new_cap;
		}

	}

	if (this->size <= this->offset) {
		this->size = this->offset + 1;
	}

	// update element count
	this->count ++;

}
