
#pragma once
#include "external.h"

typedef struct {

	const char* key;
	char* value;

	int key_length;
	int value_length;

} HttpPair;

typedef struct {

	char* header;

	HttpPair* pairs;
	int count;

} HttpHeader;

/// Helper for reading of specific information from HTTP headers
/// Returns -1 if the attempt failed, 0 otherwise.
int http_read(int connfd, HttpHeader* header, int line_buffer_size, bool read_status_line);

/// Helper for forming and sending of HTTP headers
/// Returns -1 if the attempt failed, 0 otherwise.
int http_write(int connfd, HttpHeader* header);

/// Attempts to switch protocols from HTTP to WebSockets
/// Returns -1 if the attempt failed, 0 otherwise.
int http_upgrade(int connfd);
