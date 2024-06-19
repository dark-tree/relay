
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
