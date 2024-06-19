
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

typedef struct {
	char* line;
	uint64_t length;
	uint64_t offset;
} InputLine;

/// Creates new empty line struct. This function should typically be used once
/// before calling any other input function. Use input_free() to dealocate a line struct after use.
void input_create(InputLine* line);

///
/// Free the line struct after use.
void input_free(InputLine* line);

/// Reads one line from standard input into the line struct
/// The line struct given needs not be cleard before use
void input_readline(InputLine* line);

/// Reads next token from the line into the given buffer,
/// where limit is the length in bytes of the buffer given.
bool input_token(InputLine* line, char* buffer, uint32_t limit);

/// Reads the contnet of the next string from the line into the given buffer,
/// where limit is the length in bytes of the buffer given.
bool input_string(InputLine* line, char* buffer, uint32_t limit);

/// Helper function for parsing the given c-string into a number
/// Returns true on success, on failer returns false and logs the error
bool input_parse(long* num, char* buffer);

/// Helper function for reading one number from line struct
/// Returns true on success, on failer returns false and logs the error
bool input_number(InputLine* line, long* num);
