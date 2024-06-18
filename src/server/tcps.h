
#pragma once
#include "external.h"

#include <common/stream.h>

typedef struct TcpServer_tag {

	bool shutdown;
	pthread_t thread;
	int sockfd;
	void* userdata;
	NioFunctor functor;

	void (*accept_callback) (int connfd, struct TcpServer_tag* server);
	void (*cleanup_callback) (int sockfd, struct TcpServer_tag* server);

} TcpServer;

/// Create a new TCP server and start listening for incoming
/// connection when that happens a callback is triggered, the server can be stopped with tcps_stop
void tcps_start(TcpServer* server, uint16_t port, uint16_t backlog, void* userdata);

/// Sends a stop signal to the server thread and once it stops invokes the cleanup callback.
/// That callback should eventually call close() on the given socket handle
void tcps_stop(TcpServer* server);
