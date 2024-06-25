
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

#include <common/network.h>

typedef struct {
	NetStream* base;
	NetRead read;
	NetWrite write;
	NetFree free;
	NetFlush flush;
	NetStates* net;
	const char* id;

	// private
	NetStates states;
} RawStream;

static int raw_read(NetStream* stream, void* buffer, uint32_t length) {
	RawStream* raw = (RawStream*) stream;
	return read(raw->states.connfd, buffer, length);
}

static int raw_write(NetStream* stream, void* buffer, uint32_t length) {
	RawStream* raw = (RawStream*) stream;
	return write(raw->states.connfd, buffer, length);
}

static void raw_free(NetStream* stream) {
	RawStream* raw = (RawStream*) stream;
	NetStates* states = &raw->states;

	// private
	states->open = false;
	close(states->connfd);
	sem_destroy(&states->mutex);

	net_free(stream->base);
	free(stream);
}

NetStream* net_raw(NetConsts* consts) {
	RawStream* stream = malloc(sizeof(RawStream));
	NetStates* states = &stream->states;

	// public
	stream->base = NULL;
	stream->read = raw_read;
	stream->write = raw_write;
	stream->free = raw_free;
	stream->flush = NULL;
	stream->net = states;
	stream->id = "TCP";

	// private
	states->connfd = consts->connfd;
	states->open = true;
	sem_init(&states->mutex, 0, 1);

	return (NetStream*) stream;
}
