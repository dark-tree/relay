
#pragma once
#include "external.h"

void util_mstime(struct timeval *tv, uint32_t ms);

uint32_t util_min(uint32_t a, uint32_t b);

void util_sanitize(uint8_t* buffer, uint32_t len);

