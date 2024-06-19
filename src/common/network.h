
#pragma once
#include "external.h"

#include <common/stream.h>

extern NioFunctor net_tcp;
extern NioFunctor net_ws;

/// Changes the returns value domain from [-1, N] to {-1, 0, N} - ensuring
/// that either an error is reported or the read is completed in its entirety.
int net_write(NioFunctor* base, NioStream* stream, void* buffer, int bytes);

/// Changes the returns value domain from [-1, N] to {-1, 0, N} - ensuring
/// that either an error is reported or the write is completed in its entirety.
int net_read(NioFunctor* base, NioStream* stream, void* buffer, int bytes);
