
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

typedef struct TcpServer_tag {

	bool shutdown;
	pthread_t thread;
	int sockfd;
	void* userdata;
	NetFactory factory;

	void (*accept_callback) (int connfd, struct TcpServer_tag* server);
	void (*cleanup_callback) (int sockfd, struct TcpServer_tag* server);

} TcpServer;

/// Create a new TCP server and start listening for incoming
/// connection when that happens a callback is triggered, the server can be stopped with tcps_stop
void tcps_start(TcpServer* server, uint16_t port, uint16_t backlog, void* userdata);

/// Sends a stop signal to the server thread and once it stops invokes the cleanup callback.
/// That callback should eventually call close() on the given socket handle
void tcps_stop(TcpServer* server);
