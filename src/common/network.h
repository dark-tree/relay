
#pragma once
#include "external.h"

#include <common/stream.h>

extern NioFunctor net_tcp;
extern NioFunctor net_ws;

int net_write(NioFunctor* base, NioStream* stream, void* buffer, int bytes);
int net_read(NioFunctor* base, NioStream* stream, void* buffer, int bytes);
