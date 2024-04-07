
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common/logger.h"

#define streq(a, b) (strcmp(a, b) == 0)

typedef struct {
	char* line;
	uint64_t length;
	uint64_t offset;
} InputLine;

/// Creates new empty line struct.
/// This function should typically be used once
/// before calling any other input function. 
/// Use input_free() to dealocate a line struct after use.
void input_create(InputLine* line);

/// Free the line struct after use.
void input_free(InputLine* line);

/// Reads one line from standard input into the line struct
/// The line struct given needs not be cleard before use
void input_readline(InputLine* line);

/// Reads next token from the line into the given buffer,
/// where limit is the length in bytes of the buffer given.
/// The boolean parameter 'string' controls whether the next token
/// is to be expected to be a string. The function will then write the string
/// content into the given buffer (without the quotes).
/// Returns true on success, on failer returns false and logs the error
bool input_token(InputLine* line, char* buffer, uint32_t limit, bool string);

/// Helper function for parsing the given c-string into a number
/// Returns true on success, on failer returns false and logs the error
bool input_parse(long* num, char* buffer);

/// Helper function for reading one number from line struct
/// Returns true on success, on failer returns false and logs the error
bool input_number(InputLine* line, long* num);
