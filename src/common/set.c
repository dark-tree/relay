
#include "set.h"

IdSetNode* idset_mknode(void* val, IdSetNode* next) {
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
