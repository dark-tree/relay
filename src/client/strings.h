
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

#include <common/const.h>

/// Convert a setting key from
/// integer enum to string.
const char* str_sets_decode(uint32_t key);

/// Convert a setting key from
/// string to integer enum.
int str_sets_encode(const char* key);

/// Convert a make/join command
/// response status code to human readable string.
const char* str_makes_decode(uint8_t code);

/// Convert a role enum from
/// integer to string.
const char* str_role_decode(uint8_t role);
