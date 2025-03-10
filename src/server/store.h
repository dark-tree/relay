
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

#include <common/stream.h>
#include <common/map.h>
#include <server/mutex.h>
#include <server/sequence.h>

struct User_tag;
struct Group_tag;

typedef struct IdStore_tag {

	IdMap* map;
	SharedMutex mutex;
	IdSequence sequence;
	atomic_int counter;

} IdStore;

/// Create a new synchronized storage object
/// it needs to be later freed with store_free
IdStore* store_create(IdSeqMode mode);

/// Frees memory alloced for the synchronized storage and any internally
/// used structures. The object is no longer usable after this call.
void store_free(IdStore* store);

/// Creates a new instance of a user and emplaces it into the
/// synchronized storage. Refere to the user_create function
struct User_tag* store_putuser(IdStore* store, NioStream stream);

/// Creates a new instance of a group and emplaces it into the
/// synchronized storage. Refere to the group_create function
struct Group_tag* store_putgroup(IdStore* store, struct User_tag* user);

/// Removes a user or group object of the given ID
/// from the synchronized storage
void store_remove(IdStore* store, uint32_t id);
