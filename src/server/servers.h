
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

void server_start(ServerPool* pool, int backlog, Config* cfg);
void server_stop(ServerPool* pool);
