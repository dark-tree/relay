
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

#include "util.h"

void util_mstime(struct timeval *tv, uint32_t ms) {
	tv->tv_sec = ms / 1000;
	tv->tv_usec = (ms % 1000) * 1000;
}

uint32_t util_min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

void util_sanitize(uint8_t* buffer, uint32_t len) {

	for (int i = 0; i < len; i ++) {
		uint8_t byte = buffer[i];

		if (byte == 0) {
			return;
		}

		if (byte < ' ') {
			buffer[i] = '?';
		}

		if (byte > '~') {
			buffer[i] = '?';
		}
	}

}

char* base64_encode(const unsigned char* input, int length) {
	const int pl = 4 * ((length + 2) / 3);
	char* output = calloc(pl + 1, 1);
	const int ol = EVP_EncodeBlock(output, input, length);

	if (ol != pl) {
		free(output);
		return NULL;
	}

	return output;
}
