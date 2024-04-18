
#pragma once
#include "external.h"

typedef struct TcpServer_tag {

	bool shutdown;
	pthread_t thread;
	int sockfd;
	void* userdata;

	void (*accept_callback) (int connfd, void* userdata);
	void (*cleanup_callback) (int sockfd, void* userdata);

} TcpServer;

/// Create a new TCP server and start listening for incoming
/// connection when that happens a callback is triggered, the server can be stopped with tcps_stop
void tcps_start(TcpServer* server, uint16_t port, uint16_t backlog, void* userdata);

/// Sends a stop signal to the server thread and once it stops invokes the cleanup callback.
/// That callback should eventually call close() on the given socket handle
void tcps_stop(TcpServer* server);
