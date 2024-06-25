
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

typedef struct {
	NetStream* base;
	NetRead read;
	NetWrite write;
	NetFree free;
	NetFlush flush;
	NetStates* net;
	const char* id;

	// private
	SSL* handle;
} SecureStream;

SSL_CTX* ssl_ctxinit(const char* certificate, const char* key) {
	const SSL_METHOD* method = TLS_server_method();
	SSL_CTX* ctx = SSL_CTX_new(method);

	if (!ctx) {
		log_error("Unable to create SSL context\n");
		return NULL;
	}

	if (SSL_CTX_use_certificate_file(ctx, certificate, SSL_FILETYPE_PEM) <= 0) {
		log_error("Unable to use certificate '%s'\n", certificate);
		SSL_CTX_free(ctx);
		return NULL;
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
		log_error("Unable to use private key '%s'\n", key);
		SSL_CTX_free(ctx);
		return NULL;
	}

	log_debug("Successfully created SSL context with given certificate and key\n");
	return ctx;
}

static int ssl_read(NetStream* stream, void* buffer, uint32_t length) {
	SecureStream* ssl = (SecureStream*) stream;
	return ssl->handle ? SSL_read(ssl->handle, buffer, length) : 0;
}

static int ssl_write(NetStream* stream, void* buffer, uint32_t length) {
	SecureStream* ssl = (SecureStream*) stream;
	return ssl->handle ? SSL_write(ssl->handle, buffer, length) : 0;
}

static void ssl_free(NetStream* stream) {
	SecureStream* ssl = (SecureStream*) stream;
	if (ssl->handle) {
		SSL_shutdown(ssl->handle);
		SSL_free(ssl->handle);
	}

	net_free(stream->base);
	free(stream);
}

NetStream* net_ssl(NetConsts* consts) {
	SecureStream* stream = malloc(sizeof(SecureStream));

	// public
	stream->base = NULL;
	stream->read = ssl_read;
	stream->write = ssl_write;
	stream->free = ssl_free;
	stream->flush = NULL;
	stream->id = "SSL";

	net_wrap(net_raw(consts), (NetStream*) stream);

	SSL* ssl = SSL_new(consts->sctx);
	SSL_set_fd(ssl, stream->net->connfd);

	long ret;

	if ((ret = SSL_accept(ssl)) <= 0) {
		log_debug("Failed to accept SSL connection\n");

		ret = SSL_get_error(ssl, ret);

		int try = (ret != SSL_ERROR_SSL)  && (ret != SSL_ERROR_SYSCALL) && (ret != SSL_ERROR_ZERO_RETURN);

		printf("%lu %d\n", ret, try);
		stream->net->open = false;
		SSL_free(ssl);
		ssl = NULL;
	} else {
		log_debug("Accepted SSL connection\n");
	}

	// private
	stream->handle = ssl;

	return (NetStream*) stream;
}
