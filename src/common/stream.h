
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>
#include <unistd.h>

typedef struct {
	int connfd;
	uint8_t* buffer;
	uint32_t size;
	bool open;
} NioStream;

typedef struct {
	uint32_t remaining;
	uint32_t length;
	uint8_t* buffer;
} NioBlock;

void nio_create(NioStream* stream, int connfd, uint32_t length);

void nio_free(NioStream* stream);

void nio_timeout(NioStream* stream, struct timeval* timev);

void nio_cork(NioStream* stream, int flag);

bool nio_open(NioStream* stream);

int nio_header(NioStream* stream, uint8_t* id);

NioBlock nio_block(NioStream* stream, uint32_t length);

void nio_write(NioStream* stream, void* value, uint32_t size);

void nio_write8(NioStream* stream, uint8_t value);

void nio_write16(NioStream* stream, uint16_t value);

void nio_write32(NioStream* stream, uint32_t value);

void nio_writebuf(NioStream* stream, NioBlock* block);

void nio_read(NioStream* stream, void* value, uint32_t size);

uint8_t nio_read8(NioStream* stream);

uint16_t nio_read16(NioStream* stream);

uint32_t nio_read32(NioStream* stream);

bool nio_readbuf(NioStream* stream, NioBlock* block);

void nio_skip(NioStream* stream, uint32_t bytes);

#define NIO_CORK(stream, ...) \
	nio_cork(stream, true); \
	{ \
		__VA_ARGS__ \
	} \
	nio_cork(stream, false);
