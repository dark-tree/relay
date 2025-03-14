
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

#include "network.h"

typedef struct NioStream_tag {
	uint8_t* buffer;
	uint32_t size;
	NetStream* impl;
} NioStream;

typedef struct {
	uint32_t remaining;
	uint32_t length;
	uint8_t* buffer;
} NioBlock;

/// Corectlly initializes the given NioStream pointer,
/// the length parameter controls the size of the transit buffer (the buffer used by NioBlock).
void nio_create(NioStream* stream, uint32_t length, NetStream* impl);

// Moves the stream into a 'dropped' state, while the connection is still no closed, all furture
// reads will return 0s and writes will do nothing. nio_open() will return false after this call
void nio_drop(NioStream* stream);

/// Frees the internal structures and cleanly closes the connection,
/// must by called on all NioStream structs initialized with nio_create.
void nio_free(NioStream* stream);

/// Set the read timeout, if the requsted data cannot be read in full after this time
/// the conection is considered dead, all subseqent reads and write will do nothing.
void nio_timeout(NioStream* stream, struct timeval* timev);

/// Used to either disable or enable the Nagle's algoritm, when enabled nothing will be send
/// unless enought data is gathered in the TX buffer to form a full packet.
void nio_cork(NioStream* stream, int flag);

/// Passes a 'flush' request to the underlying stream implementation
/// This is needed for streams running over WebSockets
void nio_flush(NioStream* stream);

/// Check if the connection is still usable, or did an error (including timeout) occured,
/// this should be checked often to make sure further commands make sense.
bool nio_open(NioStream* stream);

/// Correcly reads the packet header into the given variable,
/// returns true if the packet header wait loop should be broken out of.
int nio_header(NioStream* stream, uint8_t* id);

/// Creates a new NioBlock of specificed length, nio_readbuf and nio_writebuf
/// functions can then be used to copy that block from one socket to another in manageable chunks.
NioBlock nio_block(NioStream* stream, uint32_t length);

/// Writes a given number of bytes from the given buffer into the output stream,
/// for large transfers consider using nio_writebuf.
void nio_write(NioStream* stream, void* value, uint32_t size);

/// Writes next uint8_t into the output stream.
/// This method does not ensure correct endianness of the data.
void nio_write8(NioStream* stream, uint8_t value);

/// Writes next uin16_t into the output stream.
/// This method does not ensure correct endianness of the data.
void nio_write16(NioStream* stream, uint16_t value);

/// Writes next uint32_t into the output stream.
/// This method does not ensure correct endianness of the data.
void nio_write32(NioStream* stream, uint32_t value);

/// Writes a variable-length 15 bit number into the output stream.
/// This method does not ensure correct endianness of the data.
void nio_write15v(NioStream* stream, uint16_t value);

/// Writes a variable-length 32 bit number into the output stream.
/// This method does not ensure correct endianness of the data.
void nio_write32v(NioStream* stream, uint32_t value);

/// Write the next block of data from the NioBlock, this should be used with nio_readbuf
/// to transfer large buffers that cannot be copied into local memory in full at once.
void nio_writebuf(NioStream* stream, NioBlock* block);

/// Reads a given number of bytes into a given byte buffer,
/// for large transfers consider using nio_readbuf.
void nio_read(NioStream* stream, void* value, uint32_t size);

/// Reads next uint8_t from the input stream and returns it.
/// This method does not ensure correct endianness of the data.
uint8_t nio_read8(NioStream* stream);

/// Reads next uint16_t from the input stream and returns it.
/// This method does not ensure correct endianness of the data.
uint16_t nio_read16(NioStream* stream);

/// Reads next uint32_t from the input stream and returns it.
/// This method does not ensure correct endianness of the data.
uint32_t nio_read32(NioStream* stream);

/// Reads next variable-length 15 bit number from the input stream and returns it.
/// This method does not ensure correct endianness of the data.
uint16_t nio_read15v(NioStream* stream);

/// Reads next variable-length 32 bit number from the input stream and returns it.
/// This method does not ensure correct endianness of the data.
uint32_t nio_read32v(NioStream* stream);

/// Read the next block of data the NioBlock, this should be used with nio_writebuf
/// to transfer large buffers that cannot be copied into local memory in full at once.
bool nio_readbuf(NioStream* stream, NioBlock* block);

/// Ignore a section of given length of the input stream,
/// can be used to ignore invalid data in order to keep stream synchronized with the client
void nio_skip(NioStream* stream, uint32_t bytes);

// Disables Nagle's algoritm, instead TCP packets are send only when they are full
#define NIO_CORK(stream, ...) \
	nio_cork(stream, true); \
	{ \
		__VA_ARGS__ \
	} \
	nio_cork(stream, false);

// Helper macro for extracting the NetStates struct from NioStream
#define NIO_STATE(this) ((this)->impl->net)
