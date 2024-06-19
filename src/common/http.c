
#include "http.h"

#include <common/logger.h>
#include <common/util.h>
#include <common/const.h>

int http_read(NioFunctor* base, NioStream* stream, HttpHeader* http, int line_buffer_size, bool read_status_line) {

	int offset = 0;
	char buffer[line_buffer_size];
	memset(buffer, 0, line_buffer_size);

	// compute pair key lengths
	for (int i = 0; i < http->count; i ++) {

		HttpPair* pair = http->pairs + i;
		pair->key_length = strlen(pair->key);

	}

	// load header
	while (true) {

		char value;

		if (net_read(base, stream, &value, 1) <= 0) {
			return -1;
		}

		if (value == '\n') {
			if (read_status_line) {
				http->header = malloc(offset + 1);
				memcpy(http->header, buffer, offset);
				http->header[offset] = 0;
			}

			break;
		}

		// disallow appending after buffer space had run out
		if (offset >= line_buffer_size) {
			continue;
		}

		buffer[offset ++] = value;

	}

	offset = 0;
	HttpPair* pair = NULL;

	while (true) {

		char value;

		if (net_read(base, stream, &value, 1) <= 0) {
			return -1;
		}

		// let's ignore this character completely
		if (value == '\r') {
			continue;
		}

		// end of line, parse loaded data value
		if (value == '\n') {

			// the line was empty, this is the end of input
			if (offset == 0) {
				break;
			}

			// copy value into the pair
			if (pair != NULL) {
				pair->value = malloc(offset + 1);
				pair->value_length = offset;
				memcpy(pair->value, buffer, offset);
				pair->value[offset] = 0;
			}

			pair = NULL;
			offset = 0;
			continue;

		}

		if ((value == ' ') && (offset != 0) && (buffer[offset - 1] == ':')) {

			for (int i = 0; i < http->count; i ++) {

				HttpPair* hp = http->pairs + i;
				int length = offset - 1; // exclude the final ':'

				// check if the pair matches and select it
				if (hp->key_length == length && memcmp(buffer, hp->key, length) == 0) {
					pair = hp;
					break;
				}

			}

			offset = 0;
			continue;

		}

		// disallow appending after buffer space had run out
		if (offset >= line_buffer_size) {
			continue;
		}

		buffer[offset ++] = value;

	}

	return 0;

}

int http_write(NioFunctor* base, NioStream* stream, HttpHeader* http) {

	NIO_CORK(stream, {

		if (http->header != NULL) {
			net_write(base, stream, http->header, strlen(http->header));
		}

		net_write(base, stream, "\r\n", 2);

		for (int i = 0; i < http->count; i ++) {

			HttpPair* pair = http->pairs + i;

			net_write(base, stream, pair->key, strlen(pair->key));
			net_write(base, stream, ": ", 2);
			net_write(base, stream, pair->value, strlen(pair->value));
			net_write(base, stream, "\r\n", 2);

		}

		net_write(base, stream, "\r\n", 2);

	});

	return 0;

}

int http_upgrade(NioFunctor* base, NioStream* stream) {

	const char* magic_websocket_constant = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	HttpPair websocket;
	websocket.key = "Sec-WebSocket-Key";
	websocket.value = NULL;

	{
		HttpHeader http;
		http.pairs = &websocket;
		http.count = 1;

		// read http header and retrive the websocket nounce
		if (http_read(base, stream, &http, 4096, false)) {
			return -1;
		}
	}

	if (websocket.value == NULL) {
		return -1;
	}

	int magic = strlen(magic_websocket_constant);
	int total = websocket.value_length + magic;
	char merged[total + 1];

	// craft the merged string
	memcpy(merged, websocket.value, websocket.value_length);
	memcpy(merged + websocket.value_length, magic_websocket_constant, magic);
	merged[total] = 0;

	// compute solution
	unsigned char sha1[20];
	SHA1(merged, total, sha1);
	char* solution = base64_encode(sha1, 20);

	{
		HttpPair pairs[3] = {
			{"Upgrade", "websocket"},
			{"Connection", "Upgrade"},
			{"Sec-WebSocket-Accept", solution}
		};

		HttpHeader http;
		http.header = "HTTP/1.1 101 Switching Protocols";
		http.pairs = pairs;
		http.count = 3;

		if (http_write(base, stream, &http)) {
			return -1;
		}
	}

	free(solution);
	free(websocket.value);

	return 0;

}
