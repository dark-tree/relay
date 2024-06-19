
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

#include <server/sequence.h>

typedef struct Config_tag {

	// generic options
	char brand[64];
	uint32_t users;
	uint32_t level;
	uint32_t backlog;
	IdSeqMode uids;
	IdSeqMode gids;

	// server options
	uint32_t urp_port;
	uint32_t ws_port;

} Config;

typedef struct ConfigEntry_tag {

	void* value;
	bool type;

} ConfigEntry;

/// Loads default values into the given config object
/// This should be used to init the config struct before calling config_load
void config_default(Config* config);

/// Loads Entries from the given config file into
/// the given config struct, will not modify values that were not present in the file
void config_load(Config* config, const char* path);
