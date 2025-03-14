
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

/// print timestamp and log level.
/// This function should not be used manually.
void log_header(const char* type);

/// Set logging level.
/// Refer to the macros below.
void log_setlv(uint16_t bitset);

/// check if the given logging level is met
/// This function should not be used manually.
bool log_chklv(uint16_t flag);

#define LOG_FLAG_DEBUG 0x01
#define LOG_FLAG_INFO  0x02
#define LOG_FLAG_WARN  0x04
#define LOG_FLAG_ERROR 0x08
#define LOG_FLAG_FATAL 0x10

// macros for logging using the same syntax that printf uses
#define log_debug(...) if (log_chklv(LOG_FLAG_DEBUG)) { log_header("DEBUG"); printf(__VA_ARGS__); }
#define log_info(...)  if (log_chklv(LOG_FLAG_INFO))  { log_header("INFO");  printf(__VA_ARGS__); }
#define log_warn(...)  if (log_chklv(LOG_FLAG_WARN))  { log_header("WARN");  printf(__VA_ARGS__); }
#define log_error(...) if (log_chklv(LOG_FLAG_ERROR)) { log_header("ERROR"); printf(__VA_ARGS__); }
#define log_fatal(...) if (log_chklv(LOG_FLAG_FATAL)) { log_header("FATAL"); printf(__VA_ARGS__); }
