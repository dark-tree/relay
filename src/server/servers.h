
#pragma once
#include "external.h"

#include <server/config.h>
#include <server/tcps.h>

#define MAX_SERVERS 2

typedef struct {

	int max_users;
	int server_count;
	sem_t accept_mutex;
	sem_t cleanup_mutex;

	void*(*user_thread)(void*);
	TcpServer servers[MAX_SERVERS];

} ServerPool;

/// Start the server pool as specificed in the config
/// New server is made for each non-zero port number
void server_start(ServerPool* pool, Config* cfg);

/// Stops all the servers in the server pool and invokes cleanup
/// after that it terminates the application by calling exit()
void server_stop(ServerPool* pool);
