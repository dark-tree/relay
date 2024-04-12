
#include "stream.h"

#include <common/logger.h>
#include <common/util.h>

void nio_create(NioStream* stream, int connfd, uint32_t length) {
	stream->connfd = connfd;
	stream->buffer = calloc(length, 1);
	stream->size = length;
	stream->open = true;
}

void nio_drop(NioStream* stream) {
	stream->open = false;
}

void nio_free(NioStream* stream) {
	stream->open = false;
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

	if (!stream->open) {
		return 3;
	}

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

	while (stream->open) {
	
		int status = write(stream->connfd, value, size);
		
		// success
		if (status == size) {
			return;
		}
		
		// partial success, less than expected bytes actually written - retry
		if (status > 0) {
			log_debug("Call to write() completed partially! (requested: %d, got: %d)\n", size, status);
			size -= status;
			value += status;
			continue;
		}
		
		// error, consider the connection unusable
		if (status == -1) {
			log_debug("Call to write() failed! (errno: %s)\n", strerror(errno));
		
			stream->open = false;
			return;
		}
		
		// connection closed
		if (status == 0) {
			log_debug("Call to write() reported end-of-file!\n");
		
			stream->open = false;
			return;
		}
	
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

	while (stream->open) {
	
		int status = read(stream->connfd, value, size);
		
		// success
		if (status == size) {
			return;
		}
		
		// partial success, less than expected bytes actually read - retry
		if (status > 0) {
			log_debug("Call to read() completed partially! (requested: %d, got: %d)\n", size, status);
			size -= status;
			value += status;
			continue;
		}
		
		// error, consider the connection unusable
		if (status == -1) {
			log_debug("Call to read() failed! (errno: %s)\n", strerror(errno));
		
			stream->open = false;
			memset(value, 0, size);
			return;
		}
	
		// conection closed
		if (status == 0) {
			log_debug("Call to read() reported end-of-file!\n");
		
			stream->open = false;
			memset(value, 0, size);
			return;
		}
	
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
	uint32_t section = util_min(block->remaining, stream->size);
	nio_read(stream, stream->buffer, section);
	block->remaining -= section;
	block->length = section;
	return block->remaining;
}
