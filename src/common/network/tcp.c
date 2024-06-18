
#include "../network.h"

static int net_read(NioStream* stream, void* buffer, uint32_t length) {
	return read(stream->connfd, buffer, length);
}

static int net_write(NioStream* stream, void* buffer, uint32_t length) {
	return write(stream->connfd, buffer, length);
}

static int net_init(NioStream* stream) {
	// nothing to do here
}

NioFunctor net_tcp = {
	.read = net_read,
	.write = net_write,
	.init = net_init
};
