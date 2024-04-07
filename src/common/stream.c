
#include "stream.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

void nio_create(NioStream* stream, int connfd, uint32_t length) {
	stream->connfd = connfd;
	stream->buffer = calloc(length, 1);
	stream->size = length;
	stream->open = true;
}

void nio_free(NioStream* stream) {
	free(stream->buffer);
	close(stream->connfd);
}

void nio_timeout(NioStream* stream, struct timeval* timev) {
	setsockopt(stream->connfd, SOL_SOCKET, SO_RCVTIMEO, timev, sizeof(struct timeval));
}

void nio_cork(NioStream* stream, int flag) {
	setsockopt(stream->connfd, IPPROTO_TCP, TCP_CORK, &flag, sizeof(int));
}

bool nio_open(NioStream* stream) {
	return stream->open;
}

int nio_header(NioStream* stream, uint8_t* id) {
	const int status = read(stream->connfd, id, sizeof(uint8_t));

	// packet header was read
	if (status == sizeof(uint8_t)) {
		return 1;
	}

	// something other than the timeout occured, set error
	if (status == 0 || errno != EWOULDBLOCK) {
		stream->open = false;
		return 2;
	}

	// nothing was read yet
	return 0;
}

NioBlock nio_block(NioStream* stream, uint32_t length) {
	NioBlock block;

	block.remaining = length;
	block.length = 0;
	block.buffer = stream->buffer;

	return block;
}

void nio_skip(NioStream* stream, uint32_t bytes) {

	NioBlock block = nio_block(stream, bytes);

	while (block.remaining) {
		nio_readbuf(stream, &block);
	}

}

/// ---------------------- ///
///  Basic Write Functions ///
/// ---------------------- ///

void nio_write(NioStream* stream, void* value, uint32_t size) {
	if (write(stream->connfd, value, size) != size) {
		stream->open = false;
	}
}

void nio_write8(NioStream* stream, uint8_t value) {
	nio_write(stream, &value, sizeof(uint8_t));
}

void nio_write16(NioStream* stream, uint16_t value) {
	nio_write(stream, &value, sizeof(uint16_t));
}

void nio_write32(NioStream* stream, uint32_t value) {
	nio_write(stream, &value, sizeof(uint32_t));
}

void nio_writebuf(NioStream* stream, NioBlock* block) {
	nio_write(stream, block->buffer, block->length);
}

/// ---------------------- ///
///  Basic Read Functions  ///
/// ---------------------- ///

void nio_read(NioStream* stream, void* value, uint32_t size) {
	if (read(stream->connfd, value, size) != size) {
		stream->open = false;
		memset(value, 0, size);
	}
}

uint8_t nio_read8(NioStream* stream) {
	uint8_t value;
	nio_read(stream, &value, sizeof(uint8_t));
	return value;
}

uint16_t nio_read16(NioStream* stream) {
	uint16_t value;
	nio_read(stream, &value, sizeof(uint16_t));
	return value;
}

uint32_t nio_read32(NioStream* stream) {
	uint32_t value;
	nio_read(stream, &value, sizeof(uint32_t));
	return value;
}

bool nio_readbuf(NioStream* stream, NioBlock* block) {
	uint32_t section = block->remaining % stream->size;
	nio_read(stream, stream->buffer, section);
	block->remaining -= section;
	block->length = section;
	return block->remaining;
}
