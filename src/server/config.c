
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

#include "config.h"

#include <common/logger.h>

#define MAX_UINT32 0xFFFFFFFF
#define MAX_UINT16 0xFFFF

static void config_insert_idseqmode(IdSeqMode* option, const char* key, const char* val) {

	if (streq(val, "monotonic")) {
		*option = IDSEQ_MONOTONIC;
		return;
	}

	if (streq(val, "randomized")) {
		*option = IDSEQ_RANDOMIZED;
		return;
	}

	log_error("Value '%s' for key '%s' is neither 'monotonic' nor 'randomized', option ignored\n", val, key);

}

static void config_insert_long(uint32_t* option, const char* key, const char* val, uint64_t max) {

	char* endptr;
	uint64_t value = strtoull(val, &endptr, 10);

	if (value <= max) {
		*option = value;
		return;
	}

	log_error("Value '%s' for key '%s' is above the upper bound '%lu', option ignored\n", val, key, max);

}

static void config_insert_string(char* buffer, const char* key, const char* val, uint64_t max) {

	uint64_t len = strlen(val);

	if (len < 2 || val[0] != '"' || val[len - 1] != '"') {
		log_error("Value '%s' for key '%s' is not a string, option ignored\n", val, key);
		return;
	}

	if (len < max) {
		memcpy(buffer, val + 1, len - 2);
		buffer[len - 2] = 0;
		return;
	}

	log_error("Value '%s' for key '%s' is longer than the upper bound '%lu', option ignored\n", val, key, max);

}

static void config_insert(Config* cfg, const char* key, const char* val) {

	if (streq(key, "users")) {
		config_insert_long(&cfg->users, key, val, MAX_UINT32);
		return;
	}

	if (streq(key, "max-backlog")) {
		config_insert_long(&cfg->backlog, key, val, MAX_UINT16);
		return;
	}

	if (streq(key, "urp-port")) {
		config_insert_long(&cfg->urp_port, key, val, MAX_UINT16);
		return;
	}

	if (streq(key, "ws-port")) {
		config_insert_long(&cfg->ws_port, key, val, MAX_UINT16);
		return;
	}

	if (streq(key, "uids")) {
		config_insert_idseqmode(&cfg->uids, key, val);
		return;
	}

	if (streq(key, "gids")) {
		config_insert_idseqmode(&cfg->gids, key, val);
		return;
	}

	if (streq(key, "brand")) {
		config_insert_string(cfg->brand, key, val, 64);
		return;
	}

	if (streq(key, "level")) {
		uint32_t flags = LOG_FLAG_DEBUG | LOG_FLAG_INFO | LOG_FLAG_WARN | LOG_FLAG_ERROR | LOG_FLAG_FATAL;

		config_insert_long(&cfg->level, key, val, flags);
		return;
	}

	log_warn("Ignoring unknown config key '%s'\n", key);

}

void config_default(Config* config) {

	config->users = 0xFFFFFFFF;
	config->backlog = 8;
	config->urp_port = 9686;
	config->ws_port = 9687;
	config->uids = IDSEQ_MONOTONIC;
	config->gids = IDSEQ_MONOTONIC;
	config->level = LOG_FLAG_WARN | LOG_FLAG_ERROR | LOG_FLAG_FATAL;

	const char* brand = "Unknown Relay";
	memcpy(config->brand, brand, strlen(brand));

}

void config_load(Config* config, const char* path) {

	FILE* fd = fopen(path, "r");

	if (fd) {

		char key[256];
		char val[256];

		while (true) {

			const int code = fscanf(fd, "%255[^:\n]", key);

			if (code == -1) {
				break;
			}

			// skip comments
			if (key[0] == '#') {
				fscanf(fd, "\n");
				continue;
			}

			// skip empty line
			if (code == 0) {
				fscanf(fd, "\n");
				continue;
			}

			// load value
			if (code == 1) {
				fscanf(fd, "%*[: ] %255[^:\n]", val);
				config_insert(config, key, val);
				continue;
			}

		}

	}

	fclose(fd);

}
