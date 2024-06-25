
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

#include "servers.h"

#include <common/stream.h>
#include <common/network.h>
#include <common/network/ssl.h>
#include <common/logger.h>
#include <server/mutex.h>
#include <server/store.h>
#include <server/user.h>

extern IdStore* users;
extern IdStore* groups;

static void server_accept_handle(int connfd, TcpServer* server) {

	ServerPool* servers = server->userdata;

	SEMAPHORE_LOCK(&servers->accept_mutex, {

		if (users->counter >= servers->max_users) {
			close(connfd);
			return;
		}

		NetConsts consts = {0};
		consts.connfd = connfd;
		consts.sctx = servers->sctx;

		NioStream stream;
		nio_create(&stream, 0x1000, server->factory(&consts));
		User* user = store_putuser(users, stream);

		pthread_t thread;
		pthread_create(&thread, NULL, servers->user_thread, user);

	});

}

static void server_cleanup_handle(int sockfd, TcpServer* server) {

	ServerPool* servers = server->userdata;
	sem_wait(&servers->cleanup_mutex);

	// if we are NOT the last server to stop
	// this post the mutex and do nothing
	if (-- servers->server_count) {
		sem_post(&servers->cleanup_mutex);
		return;
	}

	log_info("Server shutting down...\n");

	SHARED_LOCK(&users->mutex, {
		IdMapIterator iter = idmap_iterator(users->map);

		while (idmap_has(&iter)) {
			User* user = idmap_next(&iter);
			nio_drop(&user->stream);
		}
	});

	// spin lock!
	while (users->counter > 0) {
		usleep(10000); // 10ms
	}

	// server pool cleanup
	sem_destroy(&servers->cleanup_mutex);
	sem_destroy(&servers->accept_mutex);

	// free OpenSSL context
	if (servers->sctx) {
		SSL_CTX_free(servers->sctx);
	}

	close(sockfd);
	store_free(users);
	store_free(groups);
	exit(0);

}

static void server_insert(ServerPool* pool, int port, NetFactory factory, int backlog, int* index, bool cryptographic, const char* name) {

	if (!port) {
		return;
	}

	if (cryptographic && !pool->sctx) {
		log_error("Unable to start %s server on port %d, cryptographic context required\n", name, port);
		return;
	}

	TcpServer* server = (pool->servers + *index);
	server->accept_callback = server_accept_handle;
	server->cleanup_callback = server_cleanup_handle;
	server->factory = factory;

	tcps_start(server, port, backlog, pool);
	(*index) ++;

}

void server_start(ServerPool* pool, Config* cfg) {

	// initialize the openssl cryptographic context
	pool->sctx = cfg->ssl_enable ? ssl_ctxinit(cfg->ssl_cert, cfg->ssl_key) : NULL;

	// used in accept and cleaup handles
	sem_init(&pool->accept_mutex, 0, 1);
	sem_init(&pool->cleanup_mutex, 0, 1);

	pool->max_users = cfg->users;

	int backlog = cfg->backlog;
	int index = 0;

	server_insert(pool, cfg->urp_port, net_raw, backlog, &index, false, "TCP");
	server_insert(pool, cfg->ws_port, net_ws, backlog, &index, false, "WebSocket");
	server_insert(pool, cfg->wss_port, net_wss, backlog, &index, true, "WebSocket");

	pool->server_count = index;

}

void server_stop(ServerPool* pool) {

	// don't remove! this copy is made as
	// the server_count is decremented in cleanup_handle
	// as the servers are shutting down
	const int count = pool->server_count;

	for (int i = 0; i < count; i ++) {
		tcps_stop(pool->servers + i);
	}

}
