
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

