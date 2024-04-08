
#pragma once
#include "external.h"

/// print timestamp and log level.
/// This function should not be used manually.
void log_header(const char* type);

/// Set logging level.
/// Refer to the macros below.
void log_setlevel(uint16_t bitset);

/// check if the given logging level is met
/// This function should not be used manually.
bool log_chklevel(uint16_t flag);

#define LOG_FLAG_INFO  1
#define LOG_FLAG_WARN  2
#define LOG_FLAG_ERROR 4
#define LOG_FLAG_FATAL 8

// macros for logging using the same syntax that printf uses
#define log_info(...)  if (log_chklevel(LOG_FLAG_INFO))  { log_header("INFO");  printf(__VA_ARGS__); }
#define log_warn(...)  if (log_chklevel(LOG_FLAG_WARN))  { log_header("WARN");  printf(__VA_ARGS__); }
#define log_error(...) if (log_chklevel(LOG_FLAG_ERROR)) { log_header("ERROR"); printf(__VA_ARGS__); }
#define log_fatal(...) if (log_chklevel(LOG_FLAG_FATAL)) { log_header("FATAL"); printf(__VA_ARGS__); }
