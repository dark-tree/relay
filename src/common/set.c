
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

#include "set.h"

static IdSetNode* idset_mknode(void* val, IdSetNode* next) {
	IdSetNode* node = malloc(sizeof(IdSetNode));

	node->val = val;
	node->next = next;

	return node;
}

IdSet idset_create() {
	IdSet set;

	set.length = 0;
	set.head = NULL;

	return set;
}

void idset_free(IdSet* set) {
	IdSetNode* node = set->head;

	while (node != NULL) {
		IdSetNode* tmp = node;
		node = node->next;

		free(tmp);
	}
}

void idset_put(IdSet* set, void* val) {
	set->head = idset_mknode(val, set->head);
}

IdSetIterator idset_iterator(IdSet* set) {
	IdSetIterator iterator;

	iterator.base = &(set->head);
	iterator.node = set->head;

	return iterator;
}

bool idset_has(IdSetIterator* iterator) {
	return iterator->node != NULL;
}

void* idset_next(IdSetIterator* iterator) {
	IdSetNode* node = iterator->node;

	if (node != NULL) {
		iterator->base = &(node->next);
		iterator->node = node->next;
	}

	if (node) {
		return node->val;
	}

	return NULL;
}

void* idset_peek(IdSetIterator* iterator) {
	return iterator->node->val;
}

IdSetNode* idset_remove(IdSetIterator* iterator) {
	IdSetNode* node = iterator->node;
	IdSetNode* next = node->next;

	*iterator->base = next;
	iterator->node = next;

	return node;
}
