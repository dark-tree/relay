
#pragma once
#include "external.h"

#include <server/sequence.h>

typedef struct Config_tag {

	FILE* fd;

	// generic options
	char brand[64];
	uint32_t users;
	uint32_t level;
	IdSeqMode uids;
	IdSeqMode gids;

	// urp server options
	uint32_t urp_port;

	// websocket options
	uint32_t ws_port;

} Config;

typedef struct ConfigEntry_tag {

	void* value;
	bool type;

} ConfigEntry;

void config_default(Config* config);

void config_load(Config* config, const char* path);

void config_free(Config* config);
