
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

typedef enum {
	ROLE_CONNECTED = 1,
	ROLE_MEMBER    = 2,
	ROLE_HOST      = 4
} UserRole;

typedef struct User_tag {
	uint32_t uid;
	UserRole role;
	struct Group_tag* group;
	NioStream stream;
	volatile bool exit;
} User;

/// Allocates a new user and returns a pointer to it, the user will be
/// assigned the passed in UID and NioStream
User* user_create(uint32_t uid, NioStream stream);

/// Frees memory alloced for the user and any internally
/// used structures. The object is no longer usable after this call.
void user_free(User* user);

/// Verifies if a user has any of the specificed roles
/// otherwise will return 1 and a status reminder to the user
int user_verify(User* user, uint8_t role);

/// Handles the U2R_KICK packet, must be called
/// from the thread of the user that send the packet.
void user_kick(User* user, uint32_t uid);

/// Handles the U2R_QUIT packet, must be called
/// from the thread of the user that send the packet.
void user_quit(User* user);

/// Gets a pointer to a setting given a key and check for all
/// required permissions given if a read-only or read-write access is required
uint32_t* user_setting_get(User* user, uint32_t key, bool write);

/// Sends key-value update to the
/// given user as the R2U_VALS
void user_setting_send(User* user, uint32_t key, uint32_t val);

/// semaphore lock macro helper
#define WRITE_LOCK(user, ...) SEMAPHORE_LOCK(&NIO_STATE(&(user)->stream)->mutex, __VA_ARGS__);
