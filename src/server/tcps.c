
#include "tcps.h"

#include <common/logger.h>

void tcps_dummy_handler(int signo, siginfo_t *info, void *context) {
	// do nothing, we only care that the thread was interrupted
}

void* tcps_accept(void* args) {

	TcpServer* server = args;

	struct sigaction action = {0};
	action.sa_flags = 0;
	action.sa_sigaction = tcps_dummy_handler;

	// we need to handle the SIGUSR1 in this thread so that accept() returns EINTR
	// Note: we can't use SIG_IGN here as that wouldn't interrupt the accept() call
	// Note: we have to use sigaction() over signal(), as otherwise it doesn't work (idk why)
	if (sigaction(SIGUSR1, &action, NULL) == -1) {
		log_fatal("Failed to set a dummy handler for SIGUSR1 in accept thread!\n");
		exit(-1);
	}

	while (true) {

		struct sockaddr_in address;
		int len = sizeof(address);

		int connfd = accept(server->sockfd, (struct sockaddr*) &address, &len);

		if (connfd == -1) {

			const int err = errno;

			if (err == EAGAIN || err == EWOULDBLOCK || err == ETIMEDOUT) {
				log_debug("Call to accept() timed out\n");
				continue;
			}

			if (err == EMFILE || err == ENFILE || err == ENOMEM || err == ENOBUFS || err == ENOSR) {
				log_error("Failed to accept new connection, resources exhausted!\n");
				continue;
			}

			if (err == EPERM) {
				log_error("Failed to accept new connection, operation not permitted!\n");
				continue;

			}

			if (err == EPROTO) {
				log_warn("Failed to accept new connection, protocol error!\n");
				continue;
			}

			// this is triggered from the main thread with the
			// 'stop' command (it sends a SIGUSR1 signal to this
			// thread using pthread_kill)
			if (err == EINTR && server->shutdown) {
				break;
			}

			log_fatal("Unexpected error in accept thread: %s", strerror(err));
			break;
		}

		server->accept_callback(connfd, server->userdata);

	}

	// if we got here server exit must have been triggered, or error occured
	log_info("Server shutting down...\n");
	server->cleanup_callback(server->sockfd, server->userdata);

}

void tcps_start(TcpServer* server, uint16_t port, uint16_t backlog, void* userdata) {

	struct sockaddr_in address;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		log_fatal("Failed to open socket!\n");
		exit(-1);
	}

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		log_error("Failed to configure socket address!\n");
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
		log_error("Failed to configure socket port!\n");
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*) &address, sizeof(address)) != 0) {
		log_fatal("Failed to bind to port!\n");
		exit(-1);
	}

	if (listen(sockfd, backlog) != 0) {
		log_fatal("Failed to start listening on socket!\n");
		exit(-1);
	}

	server->sockfd = sockfd;
	server->shutdown = false;
	server->userdata = userdata;

	// start the server thread, it will lisiten for incoming
	// connections and call the configured callbacks
	pthread_create(&server->thread, NULL, tcps_accept, (void*) server);

}

void tcps_stop(TcpServer* server) {
	server->shutdown = true;
	pthread_kill(server->thread, SIGUSR1);
}
