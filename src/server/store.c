
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
