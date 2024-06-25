
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

#include "stream.h"

#include <common/logger.h>
#include <common/util.h>
#include <common/const.h>

void nio_create(NioStream* stream, uint32_t length, NetStream* impl) {
	stream->buffer = calloc(length, 1);
	stream->size = length;
	stream->impl = impl;
}

void nio_drop(NioStream* stream) {
	NIO_STATE(stream)->open = false;
}

void nio_free(NioStream* stream) {
	free(stream->buffer);
	net_free(stream->impl);
}

void nio_timeout(NioStream* stream, struct timeval* timev) {
	setsockopt(NIO_STATE(stream)->connfd, SOL_SOCKET, SO_RCVTIMEO, timev, sizeof(struct timeval));
}

void nio_cork(NioStream* stream, int flag) {
	setsockopt(NIO_STATE(stream)->connfd, IPPROTO_TCP, TCP_CORK, &flag, sizeof(int));
}

void nio_flush(NioStream* stream) {
	NetFlush flush = stream->impl->flush;

	if (flush) {
		flush(stream->impl);
	}
}

bool nio_open(NioStream* stream) {
	return NIO_STATE(stream)->open;
}

int nio_header(NioStream* stream, uint8_t* id) {

	NetStream* impl = stream->impl;
	NetStates* stat = impl->net;

	if (!stat->open) {
		return 3;
	}

	const int status = impl->read(impl, id, sizeof(uint8_t));

	// packet header was read
	if (status == sizeof(uint8_t)) {
		return 1;
	}

	// something other than the timeout occured, set error
	if (status == 0 || errno != EWOULDBLOCK) {
		log_debug("Call to read() was aborted! (status: %d)\n", status);
		stat->open = false;
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

	NetStream* impl = stream->impl;
	NetStates* stat = impl->net;

	while (stat->open) {

		int status = impl->write(impl, value, size);

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
			stat->open = false;
			return;
		}

		// connection closed
		if (status == 0) {
			log_debug("Call to write() reported end-of-file!\n");
			stat->open = false;
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

	NetStream* impl = stream->impl;
	NetStates* stat = impl->net;

	while (stat->open) {

		int status = impl->read(impl, value, size);

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
			stat->open = false;
			memset(value, 0, size);
			return;
		}

		// conection closed
		if (status == 0) {
			log_debug("Call to read() reported end-of-file!\n");
			stat->open = false;
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
