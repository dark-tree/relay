
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

extern NioFunctor net_tcp;
extern NioFunctor net_ssl;
extern NioFunctor net_ws;

/// Changes the returns value domain from [-1, N] to {-1, 0, N} - ensuring
/// that either an error is reported or the read is completed in its entirety.
int net_write(NioFunctor* base, NioStream* stream, void* buffer, int bytes);

/// Changes the returns value domain from [-1, N] to {-1, 0, N} - ensuring
/// that either an error is reported or the write is completed in its entirety.
int net_read(NioFunctor* base, NioStream* stream, void* buffer, int bytes);
