
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


typedef struct IdSetNode_tag {
	void* val;
	struct IdSetNode_tag* next;
} IdSetNode;

typedef struct {
	int length;
	IdSetNode* head;
} IdSet;

typedef struct {
	IdSetNode** base;
	IdSetNode* node;
} IdSetIterator;

/// create new list, needs to be freed using idset_free()
IdSet idset_create();

/// deletes a list created with idset_create()
void idset_free(IdSet* set);

/// puts a value into the list
void idset_put(IdSet* set, void* val);

/// create a list entry iterator
IdSetIterator idset_iterator(IdSet* set);

/// check if the list iterator has any elements left
bool idset_has(IdSetIterator* iterator);

/// get next node from the list iterator
void* idset_next(IdSetIterator* iterator);

/// get node from the list iterator without advancing it
void* idset_peek(IdSetIterator* iterator);

/// removes the element pointed to by the iterator
IdSetNode* idset_remove(IdSetIterator* iterator);
