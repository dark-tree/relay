
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
