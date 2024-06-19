
#include "../network.h"

static int tcp_read(NioStream* stream, void* buffer, uint32_t length) {
	return read(stream->connfd, buffer, length);
}

static int tcp_write(NioStream* stream, void* buffer, uint32_t length) {
	return write(stream->connfd, buffer, length);
}

static int tcp_init(NioStream* stream) {
	// nothing to do here
}

static int tcp_flush(NioStream* stream) {
	// nothing to do here
}

NioFunctor net_tcp = {
	.read = tcp_read,
	.write = tcp_write,
	.flush = tcp_flush,
	.init = tcp_init
};
