
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

#pragma once
#include "external.h"

#include <common/vec.h>
#include <common/map.h>
#include <server/mutex.h>

typedef struct Group_tag {
	uint32_t gid;
	uint32_t uid;
	struct User_tag* host;
	IdVec members;

	// settings
	uint32_t password;      // group join password
	uint32_t flags;         // a collection of group flags
	uint32_t member_limit;  // maximum number of group members
	uint32_t payload_limit; // maximum packet size in this group

	volatile bool close;
	atomic_int refcnt;

	// Group Disband Mutex, this is an initially locked
	// mutex that is unlocked by the last user to leave
	// the group, that is waited on by the cleanup thread
	sem_t disband_mutex;

	// Master Group Lock, this lock is locked
	// during any modefication of the group
	// state, or when data is to be send to more than
	// one user at a time (to prevent a deadlock
	// during the locking of the user write locks)
	sem_t master_mutex;
} Group;

/// Allocates a new group and returns a pointer to it, the user will be
/// initialized so that the given user will be it's host. The given user is not modified.
Group* group_create(uint32_t gid, struct User_tag* user);

/// Find a user of the given UID and return its index in the
/// group memeber set. This method has O(n) complexity.
int group_find(Group* group, uint32_t uid);

/// Frees memory alloced for the group and any internally
/// used structures. The object is no longer usable after this call.
void group_free(Group* group);

/// Removes the given user from the given group, and handles
/// all necesery synchronization. Must be called from the user's thread.
void group_exit(Group* group, struct User_tag* user);

/// Begins a non-blocking group disbanding, after calling this mehtod no new member can join
/// the group, and all existing memebers will slowly leave. Must by called from the host's user thread.
void group_disband(Group* group);

/// Marks the user of the given UID for removal from the group. The user in
/// question will later remove itself from the group. Can be called from any user thread.
bool group_remove(Group* group, uint32_t uid);
