
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

#include <common/stream.h>
#include <common/network.h>

typedef struct {

	const char* key;
	char* value;

	int key_length;
	int value_length;

} HttpPair;

typedef struct {

	char* header;

	HttpPair* pairs;
	int count;

} HttpHeader;

/// Helper for reading of specific information from HTTP headers
/// Returns -1 if the attempt failed, 0 otherwise.
int http_read(NetStream* stream, HttpHeader* header, int line_buffer_size, bool read_status_line);

/// Helper for forming and sending of HTTP headers
/// Returns -1 if the attempt failed, 0 otherwise.
int http_write(NetStream* stream, HttpHeader* header);

/// Attempts to switch protocols from HTTP to WebSockets
/// Returns -1 if the attempt failed, 0 otherwise.
int http_upgrade(NetStream* stream);
