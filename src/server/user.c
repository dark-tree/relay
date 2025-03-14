
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

#include "user.h"

#include <common/const.h>
#include <common/logger.h>
#include <server/mutex.h>
#include <server/group.h>

User* user_create(uint32_t uid, NioStream stream) {
	User* user = malloc(sizeof(User));

	user->uid = uid;
	user->role = ROLE_CONNECTED;
	user->group = NULL;
	user->exit = false;
	user->stream = stream;

	return user;
}

void user_free(User* user) {
	nio_free(&user->stream);
	free(user);
}

int user_verify(User* user, uint8_t role) {
	if (user->role & role) {
		return 0;
	}

	NioStream* stream = &user->stream;

	WRITE_LOCK(user, {
		nio_write8(stream, R2U_STAT);
		nio_write8(stream, user->role);
	});

	return 1;
}

void user_kick(User* user, uint32_t uid) {
	Group* group = user->group;

	if (group->uid == uid) {
		group_disband(group);
		return;
	}

	if (!group_remove(group, uid)) {
		log_debug("User #%d tried to kick unassociated or non-existant user #%d", user->uid, uid);
	}

	return;
}

void user_quit(User* user) {
	Group* group = user->group;

	const uint32_t uid = user->uid;
	const uint32_t gid = user->group->gid;

	if (group->uid == uid) {
		group_disband(group);
		return;
	}

	group_exit(group, user);
	log_info("User #%d left group #%d\n", uid, gid);
	return;
}

uint32_t* user_setting_get(User* user, uint32_t key, bool write) {

	const uint8_t group_mask = (ROLE_HOST | (write ? 0 : ROLE_MEMBER));

	if (key == SETK_GROUP_PASS) {
		if (user_verify(user, group_mask)) {
			return NULL;
		}

		return &user->group->password;
	}

	if (key == SETK_GROUP_FLAGS) {
		if (user_verify(user, group_mask)) {
			return NULL;
		}

		return &user->group->flags;
	}

	if (key == SETK_GROUP_MEMBERS) {
		if (user_verify(user, group_mask)) {
			return NULL;
		}

		return &user->group->member_limit;
	}

	if (key == SETK_GROUP_PAYLOAD) {
		if (user_verify(user, group_mask)) {
			return NULL;
		}

		return &user->group->payload_limit;
	}

	// if there is no setting with the requested key
	// respond back with a invalid setting key
	user_setting_send(user, SETK_INVALID, key);

	return NULL;

}

void user_setting_send(User* user, uint32_t key, uint32_t val) {
	WRITE_LOCK(user, {
		NioStream* stream = &user->stream;

		NIO_CORK(stream, {
			nio_write8(stream, R2U_VALS);
			nio_write32(stream, key);
			nio_write32(stream, val);
		});
	});
}
