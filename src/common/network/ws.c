
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

typedef struct {

	uint64_t bytes;
	uint64_t mask;
	uint64_t offset;

	NioFunctor base;

} WebsocketState;

static int net_read(NioStream* stream, void* buffer, uint32_t bytes) {

	WebsocketState* state = (WebsocketState*) stream->super;
	NioFunctor* base = &state->base;

	int consumed = 0;
	uint8_t* mskb = (uint8_t*) &state->mask;

	if (state->bytes > 0) {
		consumed = util_min(bytes, state->bytes);

		state->bytes -= consumed;
		bytes -= consumed;

		if (base->read(stream, buffer, consumed) != consumed) {
			return -1;
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

	while (bytes > 0) {

		uint8_t head[2];

		// read websocket packet header
		if (base->read(stream, head + 0, 1) <= 0) return -1;
		if (base->read(stream, head + 1, 1) <= 0) return -1;

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
			if (base->read(stream, &blen_32, 4) <= 0) return -1;
			length = be32toh(blen_32);
		}

		if (len == 127) {
			uint64_t blen_64;
			if (base->read(stream, &blen_64, 8) <= 0) return -1;
			length = be64toh(blen_64);

			const uint64_t msb = 0x8000000000000000;

			// unset MSB as specification
			// disallows lengths with msb=1
			if (length & msb) {
				length &= (~msb);
			}
		}

		if (msk) {
			if (base->read(stream, &state->mask, 4) <= 0) return -1;
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
			base->read(stream, buffer, len);
		}

		if (opc == WEBSOCK_PING) {
			uint8_t buffer[255];
			base->read(stream, buffer, len);
		}

		if (opc == WEBSOCK_CLOSE) {
			uint8_t buffer[255];
			base->read(stream, buffer, len);
		}

	}

}

static int net_write(NioStream* stream, void* buffer, uint32_t length) {

	WebsocketState* state = (WebsocketState*) stream->super;
	NioFunctor* base = &state->base;

	int bytes = util_min(125, length);

	uint8_t v0 = (1 << 7) | WEBSOCK_BINARY;
	uint8_t v1 = (0 << 7) | bytes;

	base->write(stream, &v0, 1);
	base->write(stream, &v1, 1);

	return base->write(stream, buffer, bytes);

}

static int net_init(NioStream* stream) {
	WebsocketState* state = malloc(sizeof(WebsocketState));

	state->base = net_tcp;
	state->offset = 0;
	state->mask = 0;
	state->bytes = 0;
	stream->super = state;

	http_upgrade(stream->connfd);

}

NioFunctor net_ws = {
	.read = net_read,
	.write = net_write,
	.init = net_init
};
