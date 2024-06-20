
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

#include "../network.h"
#include "ssl.h"

#include <common/logger.h>

SSL_CTX* ssl_ctx;

static void ssl_ctxfree() {
	if (!ssl_ctx) {
		return;
	}

	SSL_CTX_free(ssl_ctx);
	ssl_ctx = NULL;
}

int ssl_ctxinit(const char* certificate, const char* key) {
	const SSL_METHOD* method = TLS_server_method();
	ssl_ctx = SSL_CTX_new(method);

	if (!ssl_ctx) {
		log_error("Unable to create SSL context!\n");
		return -1;
	}

	if (SSL_CTX_use_certificate_file(ssl_ctx, certificate, SSL_FILETYPE_PEM) <= 0) {
		log_error("Unable to use certificate '%s'!\n", certificate);
		ssl_ctxfree();
		return -1;
	}

	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key, SSL_FILETYPE_PEM) <= 0) {
		log_error("Unable to use private key '%s'!\n", key);
		ssl_ctxfree();
		return -1;
	}

	atexit(ssl_ctxfree);
	log_debug("SSL context created\n");
	return 0;
}

static int ssl_read(NioStream* stream, void* buffer, uint32_t length) {
	return SSL_read(stream->ssl, buffer, length);
}

static int ssl_write(NioStream* stream, void* buffer, uint32_t length) {
	return SSL_write(stream->ssl, buffer, length);
}

static int ssl_init(NioStream* stream) {
	SSL* ssl = SSL_new(ssl_ctx);
	SSL_set_fd(ssl, stream->connfd);

	if (SSL_accept(ssl) <= 0) {
		log_debug("Failed to accept SSL connection!\n");
		SSL_free(ssl);
		ssl = NULL;
	}

	stream->ssl = ssl;
	return 0;
}

static int ssl_free(NioStream* stream) {
	SSL_shutdown(stream->ssl);
	SSL_free(stream->ssl);
	return 0;
}

static int ssl_flush(NioStream* stream) {
	// nothing to do here
}

NioFunctor net_ssl = {
	.read = ssl_read,
	.write = ssl_write,
	.flush = ssl_flush,
	.init = ssl_init,
	.free = ssl_free
};
