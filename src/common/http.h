
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

int http_read(int connfd, HttpHeader* header, int line_buffer_size, bool read_status_line);
int http_write(int connfd, HttpHeader* header);
void http_upgrade(int connfd);
