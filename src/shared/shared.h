
#pragma once
#include <external.h>

#define ERROR_HOSTNAME  1
#define ERROR_SOCKET    2
#define ERROR_CONNECT   3
#define ERROR_UNWELCOME 4

const char* urpErrorString(int code);

void urpOpen(const char* hostname, uint16_t port);

void urpClose();
