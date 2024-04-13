
#pragma once
#include "external.h"

typedef struct TcpServer_tag {

	bool shutdown;
	pthread_t thread;
	int sockfd;

	void (*accept_callback) (int connfd);
	void (*cleanup_callback) (int sockfd);

} TcpServer;

void tcps_start(TcpServer* server, uint16_t port, uint16_t backlog);

void tcps_stop(TcpServer* server);
