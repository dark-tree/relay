
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
