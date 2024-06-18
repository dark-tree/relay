
#include "store.h"

#include <common/logger.h>
#include <server/user.h>
#include <server/group.h>

IdStore* store_create(IdSeqMode mode) {
	IdStore* store = malloc(sizeof(IdStore));

	store->counter = 0;
	store->map = idmap_create();
	mutex_init(&store->mutex);
	idseq_begin(&store->sequence, mode);

	return store;
}

void store_free(IdStore* store) {
	idmap_free(store->map);
	mutex_close(&store->mutex);
	free(store);
}

User* store_putuser(IdStore* store, NioStream stream) {

	uint32_t uid = idseq_next(&store->sequence);
	User* user = user_create(uid, stream);

	UNIQUE_LOCK(&store->mutex, {
		idmap_put(store->map, uid, user);
		store->counter ++;
	});

	return user;

}

Group* store_putgroup(IdStore* store, User* user) {

	const uint32_t gid = idseq_next(&store->sequence);
	Group* group = group_create(gid, user);

	UNIQUE_LOCK(&store->mutex, {
		idmap_put(store->map, gid, group);
		store->counter ++;
	});

	return group;

}

void store_remove(IdStore* store, uint32_t id) {

	UNIQUE_LOCK(&store->mutex, {
		IdMapNode* node = idmap_remove(store->map, id);

		if (node) {
			store->counter --;
			free(node);
		} else {
			log_error("Element with id: #%u was not found in entry map!\n", id);
		}
	});

}
