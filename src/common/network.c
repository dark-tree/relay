
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

#include "network.h"

void net_wrap(NetStream* base, NetStream* wrapper) {
	NetStream* head = wrapper;
	head->base = base;
	wrapper->net = base->net;
}

void net_free(NetStream* stream) {
	if (stream) {
		stream->free(stream);
	}
}

int net_write(NetStream* stream, void* buffer, int bytes) {

	const int total = bytes;

	while (bytes != 0) {
		const int result = stream->write(stream, buffer, bytes);

		// TODO validate
		if (result == -1 || result == 0) {
			return result;
		}

		bytes -= result;
		buffer += result;
	}

	return total;

}

int net_read(NetStream* stream, void* buffer, int bytes) {

	const int total = bytes;

	while (bytes != 0) {
		const int result = stream->read(stream, buffer, bytes);

		// TODO validate
		if (result == -1 || result == 0) {
			return result;
		}

		bytes -= result;
		buffer += result;
	}

	return total;

}
