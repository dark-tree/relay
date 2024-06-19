
#pragma once
#include "external.h"

#define IDMAP_MASK 0x0FFF
#define IDMAP_BUCKETS (IDMAP_MASK + 1)

typedef struct {
	uint32_t key;
	void* val;
} IdMapPair;

typedef struct IdMapNode_tag {
	IdMapPair pair;
	struct IdMapNode_tag* next;
} IdMapNode;

typedef struct {
	IdMapNode* buckets[IDMAP_BUCKETS];
} IdMap;

typedef struct {
	int bucket;
	IdMapNode* node;
	IdMap* map;
} IdMapIterator;

/// creates a new hashmap and returns it
/// The list needs to be freed using idmap_free().
IdMap* idmap_create();

/// Frees memory alloced for the idset using idmap_create() and any internally
/// used structures. The object is no longer usable after this call.
void idmap_free(IdMap* map);

/// Inserts a key-value pair into the map,
/// the key MUST NOT be already contained in the map.
void idmap_put(IdMap* map, uint32_t key, void* val);

/// Returns the map node asociated with the given key,
/// returns NULL if there is no such key in the map.
void* idmap_get(IdMap* map, uint32_t key);

/// Removes the map entry asociated with the given key,
/// you need to free() the returned node manually.
IdMapNode* idmap_remove(IdMap* map, uint32_t key);

/// Create a idmap entry iterator,
/// use idset_has(), and idset_next() to operate it.
IdMapIterator idmap_iterator(IdMap* map);

/// Check if the map iterator has any elements left
/// returns true if it does, false otherwise
bool idmap_has(IdMapIterator* iterator);

/// Get the next node from the idset iterator
/// returns NULL if there was no next element.
void* idmap_next(IdMapIterator* iterator);
