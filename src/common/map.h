
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

/// create new map, needs to be freed using idmap_free()
IdMap* idmap_create();

/// deletes a map created with idmap_create()
void idmap_free(IdMap* map);

/// puts a key-value pair into the map
void idmap_put(IdMap* map, uint32_t key, void* val);

/// get the map node asociated with the given key
void* idmap_get(IdMap* map, uint32_t key);

/// removes the node asociated with the given key
IdMapNode* idmap_remove(IdMap* map, uint32_t key);

/// get next node from the map iterator
void* idmap_next(IdMapIterator* iterator);

/// create a map entry iterator
IdMapIterator idmap_iterator(IdMap* map);

/// check if the iterator has any elements left
bool idmap_has(IdMapIterator* iterator);
