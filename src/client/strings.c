
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

#include "strings.h"

const char* str_sets_decode(uint32_t key) {

	if (key == SETK_INVALID) return "invalid";
	if (key == SETK_GROUP_PASS) return "group.password";
	if (key == SETK_GROUP_FLAGS) return "group.flags";
	if (key == SETK_GROUP_MEMBERS) return "group.members";
	if (key == SETK_GROUP_PAYLOAD) return "group.payload";

	return "unknown";

}

int str_sets_encode(const char* key) {

	if (streq(key, "invalid")) return SETK_INVALID;
	if (streq(key, "group.password")) return SETK_GROUP_PASS;
	if (streq(key, "group.flags")) return SETK_GROUP_FLAGS;
	if (streq(key, "group.members")) return SETK_GROUP_MEMBERS;
	if (streq(key, "group.payload")) return SETK_GROUP_PAYLOAD;

	return SETK_INVALID;

}

const char* str_makes_decode(uint8_t code) {
	if (code == 0x00) return "Created group #%d\n";
	if (code == 0x10) return "Joined group #%d\n";

	if (code & 0xF0) {
		return "Failed to join the given group!\n";
	} else {
		return "Failed to create group!\n";
	}
}

const char* str_role_decode(uint8_t role) {
	if (role == 1) return "connected";
	if (role == 2) return "member";
	if (role == 4) return "host";

	return "<invalid value>";
}
