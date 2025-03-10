
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

#pragma once
#include "external.h"

struct NetStream_tag;
struct NetConsts_tag;

typedef int (*NetRead) (struct NetStream_tag*, void*, uint32_t);
typedef int (*NetWrite) (struct NetStream_tag*, void*, uint32_t);
typedef void (*NetFree) (struct NetStream_tag*);
typedef int (*NetFlush) (struct NetStream_tag*);
typedef struct NetStream_tag* (*NetFactory) (struct NetConsts_tag*);

typedef struct NetConsts_tag {
	int connfd;
	SSL_CTX* sctx;
} NetConsts;

typedef struct NetStates_tag {
	int connfd;
	bool open;

	// This mutex is used to guard agains two threads
	// writing at the same time to the same connection.
	// To protect against a deadlock during locking of those
	// mutexes the Master Group Lock is used, learn more in group.h
	sem_t mutex;
} NetStates;

typedef struct NetStream_tag {
	struct NetStream_tag* base; // the base of this stream struct
	NetRead read;               // stream read function
	NetWrite write;             // stream write function
	NetFree free;               // called on cleanup, from top to bottom
	NetFlush flush;             // nullable pointer to a function used for flushing any write buffers
	NetStates* net;             // pointer to the NetStates struct
	const char* id;             // human readable stream identifier
} NetStream;

NetStream* net_raw(NetConsts* consts);
NetStream* net_ws(NetConsts* consts);
NetStream* net_ssl(NetConsts* consts);
NetStream* net_wss(NetConsts* consts);

/// Wraps the given base stream in the wrapper
/// so that using read/write in the wrapper stream will invoke read/write in base
void net_wrap(NetStream* base, NetStream* wrapper);

/// Dealocates the linked streams, should be called on the top level wrapper
/// it will then recursively free all the linked bases
void net_free(NetStream* stream);

/// Changes the returns value domain from [-1, N] to {-1, 0, N} - ensuring
/// that either an error is reported or the read is completed in its entirety.
int net_write(NetStream* stream, void* buffer, int bytes);

/// Changes the returns value domain from [-1, N] to {-1, 0, N} - ensuring
/// that either an error is reported or the write is completed in its entirety.
int net_read(NetStream* stream, void* buffer, int bytes);
