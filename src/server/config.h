
#pragma once
#include "external.h"

#include <server/sequence.h>

typedef struct Config_tag {

	FILE* fd;
	
	// int options
	uint32_t users;
	uint32_t port;
	uint32_t level;
	IdSeqMode uids;
	IdSeqMode gids;
	
	// string options
	char brand[64];

} Config;

typedef struct ConfigEntry_tag {

	void* value;
	bool type;

} ConfigEntry;

void config_default(Config* config);

void config_load(Config* config, const char* path);

void config_free(Config* config);
