
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

	// input
	uint64_t bytes;
	uint64_t mask;
	uint64_t offset;

	// output
	uint64_t position;
	uint8_t buffer[WRITE_BUFFER];

	NioFunctor base;

} WebsocketState;

static int ws_send(NioStream* stream, uint8_t opcode, void* buffer, uint32_t length) {

	NioFunctor* base = &((WebsocketState*) stream->super)->base;

	int result = 0;
	uint8_t head[2] = {0};
	uint8_t len = length;

	if (length > 0xFFFF) len = 127; else
	if (length > 125) len = 126;

	head[0] = (1 << 7) | (opcode & 0x0F);
	head[1] = (0 << 7) | (len & 0x7F);

	if ((result = net_write(base, stream, head, 2)) <= 0) {
		return result;
	}

	if (len == 126) {
		uint16_t be = htobe32(length);
		if ((result = net_write(base, stream, &be, 4)) <= 0) return result;
	}

	if (len == 127) {
		uint32_t be = htobe64(length);
		if ((result = net_write(base, stream, &be, 8)) <= 0) return result;
	}

	return net_write(base, stream, buffer, length);

}

static int ws_read(NioStream* stream, void* buffer, uint32_t bytes) {

	WebsocketState* state = (WebsocketState*) stream->super;
	NioFunctor* base = &state->base;

	int result = 0;
	int consumed = 0;
	uint8_t* mskb = (uint8_t*) &state->mask;

	while (bytes > 0) {

		if (state->bytes > 0) {
			consumed = util_min(bytes, state->bytes);

			state->bytes -= consumed;
			bytes -= consumed;

			if (net_read(base, stream, buffer, consumed) <= 0) {
				return 0;
			}

			if (state->mask != 0) {
				for (int i = 0; i < consumed; i ++) {
					((uint8_t*) buffer)[i] ^= mskb[(state->offset ++) % 4];
				}
			}

			if (bytes == 0) {
				return consumed;
			}

			buffer += consumed;
		}

		uint8_t head[2];

		// read websocket packet header
		if ((result = net_read(base, stream, head, 2)) <= 0) {
			return result;
		}

		uint64_t length = 0;
		state->mask = 0;

		uint8_t fin = (head[0] & 0x80) >> 7;
		uint8_t opc = (head[0] & 0x0f) >> 0;
		uint8_t msk = (head[1] & 0x80) >> 7;
		uint8_t len = (head[1] & 0x7f) >> 0;

		if (len <= 125) {
			length = len;
		}

		if (len == 126) {
			uint32_t blen_32;
			if ((result = net_read(base, stream, &blen_32, 4)) <= 0) return result;
			length = be32toh(blen_32);
		}

		if (len == 127) {
			uint64_t blen_64;
			if ((result = net_read(base, stream, &blen_64, 8)) <= 0) return result;
			length = be64toh(blen_64);

			const uint64_t msb = 0x8000000000000000;

			// unset MSB as specification
			// disallows lengths with msb=1
			if (length & msb) {
				length &= (~msb);
			}
		}

		if (msk) {
			if ((result = net_read(base, stream, &state->mask, 4)) <= 0) return result;
		}

		// we got a fragment, text or data
		// next call to this function will successfuly read some data
		if (opc == WEBSOCK_FRAGMENT || opc == WEBSOCK_TEXT || opc == WEBSOCK_BINARY) {
			state->offset = 0;
			state->bytes = length;
			return consumed;
		}

		// is this even valid to be received by the server?
		if (opc == WEBSOCK_PONG) {
			uint8_t buffer[255];
			if ((result = net_read(base, stream, buffer, len)) <= 0) return result;
		}

		if (opc == WEBSOCK_PING) {
			uint8_t buffer[255];
			if ((result = net_read(base, stream, buffer, len)) <= 0) return result;

			// only respond if we don't have to wait, otherwise
			// we could cause a deadlock
			if (sem_trywait(&stream->write_mutex) == 0) {
				ws_send(stream, WEBSOCK_PONG, buffer, len);
				sem_post(&stream->write_mutex);
			}
		}

		if (opc == WEBSOCK_CLOSE) {
			uint8_t buffer[255];
			if ((result = net_read(base, stream, buffer, len)) <= 0) return result;

			// only respond if we don't have to wait, otherwise
			// we could cause a deadlock
			if (sem_trywait(&stream->write_mutex) == 0) {
				ws_send(stream, WEBSOCK_PONG, buffer, len);
				sem_post(&stream->write_mutex);
			}

			// kill the connection
			return 0;
		}

	}

	// calling read with 0 bytes to read is invalid
	return 0;

}

static int ws_write(NioStream* stream, void* buffer, uint32_t length) {

	WebsocketState* state = (WebsocketState*) stream->super;
	const uint32_t total = length;

	while (length > 0) {

		int result = 0;
		int remaining = WRITE_BUFFER - state->position;
		int transfered = util_min(remaining, length);

		if (remaining > 0) {
			memcpy(state->buffer + state->position, buffer, transfered);

			length -= transfered;
			remaining -= transfered;
			state->position += transfered;
		}

		if (remaining <= 0) {
			if ((result = ws_send(stream, WEBSOCK_BINARY, state->buffer, state->position)) <= 0) {
				return result;
			}

			state->position = 0;
		}

	}

	return total;

}

static int ws_flush(NioStream* stream) {

	int result = 1;
	WebsocketState* state = (WebsocketState*) stream->super;

	if (state->position == 0) {
		return 1;
	}

	sem_wait(&stream->write_mutex);
	uint32_t length = state->position;

	if (length > 0) {
		state->position = 0;
		result = ws_send(stream, WEBSOCK_BINARY, state->buffer, length);
	}

	sem_post(&stream->write_mutex);
	return result;

}

static int ws_init(NioStream* stream) {

	WebsocketState* state = malloc(sizeof(WebsocketState));

	state->base = net_tcp;
	state->offset = 0;
	state->mask = 0;
	state->bytes = 0;
	state->position = 0;
	stream->super = state;

	http_upgrade(stream->connfd);

}

NioFunctor net_ws = {
	.read = ws_read,
	.write = ws_write,
	.flush = ws_flush,
	.init = ws_init
};
