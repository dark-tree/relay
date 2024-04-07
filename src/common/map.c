
#include "map.h"

void* idmap_unpack(IdMapNode* node) {
	return node != NULL ? node->pair.val : NULL;
}

IdMapNode* idmap_mknode(uint32_t key, void* val, IdMapNode* next) {
    IdMapNode* node = malloc(sizeof(IdMapNode));

	node->pair.key = key;
	node->pair.val = val;
	node->next = next;

    return node;
}

IdMap* idmap_create() {
	IdMap* map = malloc(sizeof(IdMap));

	for (int i = 0; i < IDMAP_BUCKETS; i ++) {
		map->buckets[i] = NULL;
	}

	return map;
}

void idmap_free(IdMap* map) {
	for (int i = 0; i < IDMAP_BUCKETS; i ++) {
		IdMapNode* node = map->buckets[i];

		while (node != NULL) {
			IdMapNode* tmp = node;
			node = node->next;

			free(tmp);
		}
	}

	free(map);
}

void idmap_put(IdMap* map, uint32_t key, void* val) {
	uint32_t hash = key & IDMAP_MASK;
	IdMapNode* node = map->buckets[hash];
	map->buckets[hash] = idmap_mknode(key, val, node);
}

void* idmap_get(IdMap* map, uint32_t key) {
	IdMapNode* node = map->buckets[key & IDMAP_MASK];

	while (node != NULL) {
		if (node->pair.key == key) {
			return idmap_unpack(node);
		}

		node = node->next;
	}

	return NULL;
}


IdMapNode* idmap_remove(IdMap* map, uint32_t key) {
	IdMapNode* node = map->buckets[key & IDMAP_MASK];

	if (node == NULL) {
		return NULL;
	}

	// special case for the first node
	if (node->pair.key == key) {
		map->buckets[key & IDMAP_MASK] = node->next;
		return node;
	}

	while (true) {
		IdMapNode* prev = node;
		node = node->next;

		if (node == NULL) {
			break;
		}

		if (node->pair.key == key) {
			prev->next = node->next;
			return node;
		}
	}

	return NULL;
}

void* idmap_next(IdMapIterator* iterator) {
	IdMapNode* node = iterator->node;

	if (node != NULL) {
		iterator->node = node->next;
	}

	while (iterator->node == NULL && iterator->bucket < IDMAP_MASK) {
		iterator->node = iterator->map->buckets[++ iterator->bucket];
	}

	return idmap_unpack(node);
}

IdMapIterator idmap_iterator(IdMap* map) {
	IdMapIterator iterator;

	iterator.bucket = 0;
	iterator.node = map->buckets[0];
	iterator.map = map;

	idmap_next(&iterator);

	return iterator;
}

bool idmap_has(IdMapIterator* iterator) {
	return iterator->bucket < IDMAP_MASK;
}
