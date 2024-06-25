
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

#include "../network.h"

#include <common/util.h>
#include <common/http.h>

// websocket packet identifiers
#define WEBSOCK_FRAGMENT 0
#define WEBSOCK_TEXT     1
#define WEBSOCK_BINARY   2
#define WEBSOCK_CLOSE    8
#define WEBSOCK_PING     9
#define WEBSOCK_PONG     10

#define WRITE_BUFFER 4096

typedef struct {
	NetStream* base;
	NetRead read;
	NetWrite write;
	NetFree free;
	NetFlush flush;
	NetStates* net;
	const char* id;

	// private
	uint64_t bytes;
	uint64_t mask;
	uint64_t offset;
	uint64_t position;
	uint8_t buffer[WRITE_BUFFER];
} WebsocketStream;

static int ws_send(NetStream* stream, uint8_t opcode, void* buffer, uint32_t length) {

	WebsocketStream* ws = (WebsocketStream*) stream;
	NetStream* base = stream->base;

	int result = 0;
	uint8_t head[2] = {0};
	uint8_t len = length;

	if (length > 0xFFFF) len = 127; else
	if (length > 125) len = 126;

	head[0] = (1 << 7) | (opcode & 0x0F);
	head[1] = (0 << 7) | (len & 0x7F);

	if ((result = net_write(base, head, 2)) <= 0) {
		return result;
	}

	if (len == 126) {
		uint16_t be = htobe32(length);
		if ((result = net_write(base, &be, 4)) <= 0) return result;
	}

	if (len == 127) {
		uint32_t be = htobe64(length);
		if ((result = net_write(base, &be, 8)) <= 0) return result;
	}

	return net_write(base, buffer, length);
}

static int ws_read(NetStream* stream, void* buffer, uint32_t bytes) {

	WebsocketStream* ws = (WebsocketStream*) stream;
	NetStream* base = stream->base;

	int result = 0;
	int consumed = 0;
	uint8_t* mskb = (uint8_t*) &ws->mask;

	while (bytes > 0) {

		if (ws->bytes > 0) {
			consumed = util_min(bytes, ws->bytes);

			ws->bytes -= consumed;
			bytes -= consumed;

			if (net_read(base, buffer, consumed) <= 0) {
				return 0;
			}

			if (ws->mask != 0) {
				for (int i = 0; i < consumed; i ++) {
					((uint8_t*) buffer)[i] ^= mskb[(ws->offset ++) % 4];
				}
			}

			if (bytes == 0) {
				return consumed;
			}

			buffer += consumed;
		}

		uint8_t head[2];

		// read websocket packet header
		if ((result = net_read(base, head, 2)) <= 0) {
			return result;
		}

		uint64_t length = 0;
		ws->mask = 0;

		uint8_t fin = (head[0] & 0x80) >> 7;
		uint8_t opc = (head[0] & 0x0f) >> 0;
		uint8_t msk = (head[1] & 0x80) >> 7;
		uint8_t len = (head[1] & 0x7f) >> 0;

		if (len <= 125) {
			length = len;
		}

		if (len == 126) {
			uint32_t blen_32;
			if ((result = net_read(base, &blen_32, 4)) <= 0) return result;
			length = be32toh(blen_32);
		}

		if (len == 127) {
			uint64_t blen_64;
			if ((result = net_read(base, &blen_64, 8)) <= 0) return result;
			length = be64toh(blen_64);

			const uint64_t msb = 0x8000000000000000;

			// unset MSB as specification
			// disallows lengths with msb=1
			if (length & msb) {
				length &= (~msb);
			}
		}

		if (msk) {
			if ((result = net_read(base, &ws->mask, 4)) <= 0) return result;
		}

		// we got a fragment, text or data
		// next call to this function will successfuly read some data
		if (opc == WEBSOCK_FRAGMENT || opc == WEBSOCK_TEXT || opc == WEBSOCK_BINARY) {
			ws->offset = 0;
			ws->bytes = length;
			return consumed;
		}

		// is this even valid to be received by the server?
		if (opc == WEBSOCK_PONG) {
			uint8_t buffer[255];
			if ((result = net_read(base, buffer, len)) <= 0) return result;
		}

		if (opc == WEBSOCK_PING) {
			uint8_t buffer[255];
			if ((result = net_read(base, buffer, len)) <= 0) return result;

			// only respond if we don't have to wait, otherwise
			// we could cause a deadlock
			if (sem_trywait(&ws->net->mutex) == 0) {
				ws_send(stream, WEBSOCK_PONG, buffer, len);
				sem_post(&ws->net->mutex);
			}
		}

		if (opc == WEBSOCK_CLOSE) {
			uint8_t buffer[255];
			if ((result = net_read(base, buffer, len)) <= 0) return result;

			// only respond if we don't have to wait, otherwise
			// we could cause a deadlock
			if (sem_trywait(&ws->net->mutex) == 0) {
				ws_send(stream, WEBSOCK_PONG, buffer, len);
				sem_post(&ws->net->mutex);
			}

			// kill the connection
			return 0;
		}

	}

	// calling read with 0 bytes to read is invalid
	return 0;

}

static int ws_write(NetStream* stream, void* buffer, uint32_t length) {

	WebsocketStream* ws = (WebsocketStream*) stream;
	const uint32_t total = length;

	while (length > 0) {

		int result = 0;
		int remaining = WRITE_BUFFER - ws->position;
		int transfered = util_min(remaining, length);

		if (remaining > 0) {
			memcpy(ws->buffer + ws->position, buffer, transfered);

			length -= transfered;
			remaining -= transfered;
			ws->position += transfered;
		}

		if (remaining <= 0) {
			if ((result = ws_send(stream, WEBSOCK_BINARY, ws->buffer, ws->position)) <= 0) {
				return result;
			}

			ws->position = 0;
		}

	}

	return total;

}

static int ws_flush(NetStream* stream) {

	int result = 1;
	WebsocketStream* ws = (WebsocketStream*) stream;

	if (ws->position == 0) {
		return 1;
	}

	sem_wait(&stream->net->mutex);
	uint32_t length = ws->position;

	if (length > 0) {
		ws->position = 0;
		result = ws_send(stream, WEBSOCK_BINARY, ws->buffer, length);
	}

	sem_post(&stream->net->mutex);
	return result;

}

static void ws_free(NetStream* stream) {
	net_free(stream->base);
	free(stream);
}

static NetStream* ws_create(NetConsts* consts, NetStream* base) {
	WebsocketStream* stream = malloc(sizeof(WebsocketStream));

	// public
	stream->read = ws_read;
	stream->write = ws_write;
	stream->free = ws_free;
	stream->flush = ws_flush;
	stream->id = "WebSocket";

	// private
	stream->offset = 0;
	stream->mask = 0;
	stream->bytes = 0;
	stream->position = 0;

	net_wrap(base, (NetStream*) stream);
	http_upgrade(stream->base);

	return (NetStream*) stream;
}

NetStream* net_ws(NetConsts* consts) {
	return ws_create(consts, net_raw(consts));
}

NetStream* net_wss(NetConsts* consts) {
	return ws_create(consts, net_ssl(consts));
}
