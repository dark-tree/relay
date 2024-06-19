
#pragma once
#include "external.h"

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

/// creates a new linked list and returns it
/// The list needs to be freed using idset_free().
IdSet idset_create();

/// Frees memory alloced for the idset using idset_create() and any internally
/// used structures. The object is no longer usable after this call.
void idset_free(IdSet* set);

/// Puts a new non-null value at the
/// head (beginning) of the collection.
void idset_put(IdSet* set, void* val);

/// Create a idset entry iterator,
/// use idset_has(), idset_next(), and idset_peek() to operate it.
IdSetIterator idset_iterator(IdSet* set);

/// Check if the list iterator has any elements left
/// returns true if it does, false otherwise
bool idset_has(IdSetIterator* iterator);

/// Get the next node from the idset iterator
/// returns NULL if there was no next element.
void* idset_next(IdSetIterator* iterator);

/// Get the current value from the idset iterator
/// without advancing to the next element.
void* idset_peek(IdSetIterator* iterator);

/// Remove the element pointed to by the iterator
/// you need to free() the returned node manually.
IdSetNode* idset_remove(IdSetIterator* iterator);
