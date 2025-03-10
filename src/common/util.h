
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

/// Converts miliseconds to the
/// standard timeval structure.
void util_mstime(struct timeval *tv, uint32_t ms);

/// Returns the smallest of
/// the two given integers.
uint32_t util_min(uint32_t a, uint32_t b);

/// Removes all non-printable characters from the given buffer
/// making sure it will not break formating or contain escape codes.
void util_sanitize(uint8_t* buffer, uint32_t len);

/// Returns a newly allocated c-string containing the base64 encoded
/// version of the given input buffer.
char* base64_encode(const unsigned char* input, int length);
