/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gck-rpc-tls-psk.c - TLS-PSK functionality to protect communication

   Copyright (C) 2013, NORDUnet A/S

   pkcs11-proxy is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   pkcs11-proxy is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Fredrik Thulin <fredrik@thulin.net>
*/

#include "config.h"

#include "gck-rpc-private.h"
#include "gck-rpc-tls-psk.h"

#include <sys/param.h>
#include <assert.h>

/* for file I/O */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* TLS pre-shared key */
static char tls_psk_identity[128] = { 0, };
static char tls_psk_key_filename[MAXPATHLEN] = { 0, };

/* -----------------------------------------------------------------------------
 * LOGGING and DEBUGGING
 */
#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif
#if DEBUG_OUTPUT
#define debug(x) gck_rpc_debug x
#else
#define debug(x)
#endif
#define warning(x) gck_rpc_warn x


/* -----------------------------------------------------------------------------
 * TLS-PSK (pre-shared key) functionality
 */

/* Utility function to decode a single hex char.
 *
 * Returns value as integer, or -1 on invalid hex char (not 0-9, a-f or A-F).
 */
static int
_tls_psk_to_hex(char val)
{
	if (val >= '0' && val <= '9')
		return val - '0';
	if (val >= 'a' && val <= 'f')
		return val - 'a' + 10;
	if (val >= 'A' && val <= 'F')
		return val - 'A' + 10;
	return -1;
}

/* Hex decode the key from an entry in the TLS-PSK key file. Entrys are of the form
 *
 *   identity:hex-key\n
 *
 * Logging debug/error messages here is a bit problematic since the key is sensitive
 * and should not be logged to syslog for example. This code avoids logging the key
 * part and only logs identity.
 *
 * Returns 0 on failure, number of bytes in hex-decoded key on success.
 */
static int
_tls_psk_decode_key(const char *identity, const char *hexkey, unsigned char *psk, unsigned int max_psk_len)
{
	int psk_len, i;

	/* check that length of the key is even */
	if ((strlen(hexkey) % 2) != 0) {
		warning(("un-even length TLS-PSK key"));
		return 0;
	}

	memset(psk, 0, max_psk_len);
	psk_len = 0;

	while (*hexkey && (psk_len < max_psk_len)) {
		/* decode first half of byte, check for errors */
		if ((i = _tls_psk_to_hex(*hexkey)) < 0) {
			warning(("bad TLS-PSK '%.100s' hex char at position %i (%c)",
				 identity, psk_len + 1, *hexkey));
			return 0;
		}
		*psk = i << 4;
		hexkey++;

		/* decode second half of byte, check for errors */
		if ((i = _tls_psk_to_hex(*hexkey)) < 0) {
		        warning(("bad TLS-PSK '%.100s' hex char at position %i (%c)",
				 identity, psk_len + 1, *hexkey));
			return 0;
		}
		*psk |= i;
		hexkey++;

		psk_len++;
		psk++;
	}
	if (*hexkey) {
		warning(("too long TLS-PSK '%.100s' key (max %i)", identity, max_psk_len));
		return 0;
	}

	return psk_len;
}

/*
 * Read from a file descriptor until a newline is spotted.
 *
 * Using open() and _fgets() instead of fopen() and fgets() avoids having to
 * seccomp-allow the mmap() syscall.
 *
 * Reading one byte at a time is perhaps not optimal from a performance
 * standpoint, but the kernel will surely have pre-buffered the data anyways.
 */
int _fgets(char *buf, unsigned int len, const int fd)
{
	int bytes;

	bytes = 0;

	while (len) {
		if (read(fd, buf, 1) != 1)
			break;
		bytes++;
		if (*buf == '\n') {
			buf++;
			len--;
			break;
		}
		buf++;
		len--;
	}

	if (! len)
		/* ran out of space */
		return -1;
	*buf = '\0';
	return bytes;
}

/*
 * Callbacks invoked by OpenSSL PSK initialization.
 */


/* Server side TLS-PSK initialization callback. Given an identity (chosen by the client),
 * locate a pre-shared key and put it in psk.
 *
 * Returns the number of bytes put in psk, or 0 on failure.
 */
static unsigned int
_tls_psk_server_cb(SSL *ssl, const char *identity,
		   unsigned char *psk, unsigned int max_psk_len)
{
	char line[1024], *hexkey;
	unsigned int psk_len;
	int i, fd;

	debug(("Initializing TLS-PSK with keyfile '%.100s', identity '%.100s'",
	       tls_psk_key_filename, identity));

	if ((fd = open(tls_psk_key_filename, O_RDONLY | O_CLOEXEC)) < 0) {
		gck_rpc_warn("can't open TLS-PSK keyfile '%.100s' for reading : %s",
			     tls_psk_key_filename, strerror(errno));
		return 0;
	}

	/* Format of PSK file is that of GnuTLS psktool.
	 *
	 * identity:hex-key
	 * other:another-hex-key
	*/
	psk_len = 0;

	while (_fgets(line, sizeof(line) - 1, fd) > 0) {
		/* Find first colon and replace it with NULL */
		hexkey = strchr(line, ':');
		if (! hexkey)
			continue;
		*hexkey = 0;
		hexkey++;

		/* Remove newline(s) at the end */
		for (i = strlen(hexkey) - 1; i && (hexkey[i] == '\n' || hexkey[i] == '\r'); i--)
			hexkey[i] = 0;

		if (identity == NULL || ! identity[0] || ! strcmp(line, identity)) {
			/* If the line starts with identity: or identity is not provided, parse this line. */
			psk_len = _tls_psk_decode_key(line, hexkey, psk, max_psk_len);
			if (psk_len)
				debug(("Loaded TLS-PSK '%.100s' from keyfile '%.100s'",
				       line, tls_psk_key_filename));
			else
				warning(("Failed loading TLS-PSK '%.100s' from keyfile '%.100s'",
					 line, tls_psk_key_filename));
			break;
		}
	}
	close(fd);

	return psk_len;
}

/* Client side TLS-PSK initialization callback. Indicate to OpenSSL what identity to
 * use, and the pre-shared key for that identity.
 *
 * Returns the number of bytes put in psk, or 0 on failure.
 */
static unsigned int
_tls_psk_client_cb(SSL *ssl, const char *hint,
		   char *identity, unsigned int max_identity_len,
		   unsigned char *psk, unsigned int max_psk_len)
{
	/* Client tells server which identity it wants to use in ClientKeyExchange */
	snprintf(identity, max_identity_len, "%s", tls_psk_identity);

	/* We currently just discard the hint sent to us by the server */
	return _tls_psk_server_cb(ssl, identity, psk, max_psk_len);
}


/* Initialize OpenSSL and create an SSL CTX. Should be called just once.
 *
 * Returns 0 on failure and 1 on success.
 */
int
gck_rpc_init_tls_psk(GckRpcTlsPskState *state, const char *key_filename,
		     const char *identity, enum gck_rpc_tls_psk_caller caller)
{
	char *tls_psk_ciphers = PKCS11PROXY_TLS_PSK_CIPHERS;

	if (state->initialized == 1) {
		warning(("TLS state already initialized"));
		return 0;
	}

	/* Global OpenSSL initialization */
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_ssl_algorithms();

	assert(caller == GCK_RPC_TLS_PSK_CLIENT || caller == GCK_RPC_TLS_PSK_SERVER);

	state->ssl_ctx = SSL_CTX_new(TLSv1_2_method());

	if (state->ssl_ctx == NULL) {
		gck_rpc_warn("can't initialize SSL_CTX");
		return 0;
	}

	/* Set up callback for TLS-PSK initialization */
	if (caller == GCK_RPC_TLS_PSK_CLIENT)
		SSL_CTX_set_psk_client_callback(state->ssl_ctx, _tls_psk_client_cb);
	else
		SSL_CTX_set_psk_server_callback(state->ssl_ctx, _tls_psk_server_cb);

	/* Disable compression, for security (CRIME Attack). */
	SSL_CTX_set_options(state->ssl_ctx, SSL_OP_NO_COMPRESSION);

	/* Specify ciphers to use */
	SSL_CTX_set_cipher_list(state->ssl_ctx, tls_psk_ciphers);

	snprintf(tls_psk_key_filename, sizeof(tls_psk_key_filename), "%s", key_filename);
	snprintf(tls_psk_identity, sizeof(tls_psk_identity), "%s", identity ? identity : "");

	state->type = caller;
	state->initialized = 1;

	debug(("Initialized TLS-PSK %s", caller == GCK_RPC_TLS_PSK_CLIENT ? "client" : "server"));

	return 1;
}

/* Set up SSL for a new socket. Call this after accept() or connect().
 *
 * When a socket has been created, call gck_rpc_start_tls() with the TLS state
 * initialized using gck_rpc_init_tls_psk() and the new socket.
 *
 * Returns 1 on success and 0 on failure.
 */
int
gck_rpc_start_tls(GckRpcTlsPskState *state, int sock)
{
	int res;
	char buf[256];

	state->ssl = SSL_new(state->ssl_ctx);
	if (! state->ssl) {
		warning(("can't initialize SSL"));
		return 0;
	}

	state->bio = BIO_new_socket(sock, BIO_NOCLOSE);
	if (! state->bio) {
		warning(("can't initialize SSL BIO"));
		return 0;
	}

	SSL_set_bio(state->ssl, state->bio, state->bio);

	/* Set up callback for TLS-PSK initialization */
	if (state->type == GCK_RPC_TLS_PSK_CLIENT)
		res = SSL_connect(state->ssl);
	else
		res = SSL_accept(state->ssl);

	if (res != 1) {
		ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
		warning(("can't start TLS : %i/%i (%s perhaps)",
			 res, SSL_get_error(state->ssl, res), strerror(errno)));
		warning(("SSL ERR: %s", buf));
		return 0;
	}

	return 1;
}

/* Un-initialize everything SSL related. Call this on application shut down.
 */
void
gck_rpc_close_tls(GckRpcTlsPskState *state)
{
	if (state->ssl_ctx) {
		SSL_CTX_free(state->ssl_ctx);
		state->ssl_ctx = NULL;
	}

	if (state->ssl) {
		SSL_free(state->ssl);
		state->ssl = NULL;
	}
}

/* Send data using SSL.
 *
 * Returns the number of bytes written.
 */
int
gck_rpc_tls_write_all(GckRpcTlsPskState *state, void *data, unsigned int len)
{
	int bytes, error;
	char buf[256];

	assert(state);
	assert(data);
	assert(len > 0);

	bytes = SSL_write(state->ssl, data, len);

	if (bytes <= 0) {
		while ((error = ERR_get_error())) {
			ERR_error_string_n(error, buf, sizeof(buf));
			warning(("SSL_write error: %s", buf));
		}
		return 0;
	}

	return bytes;
}

/* Read data using SSL.
 *
 * Returns the number of bytes read.
 */
int
gck_rpc_tls_read_all(GckRpcTlsPskState *state, void *data, unsigned int len)
{
	int bytes, error;
	char buf[256];

	assert(state);
	assert(data);
	assert(len > 0);

	bytes = SSL_read(state->ssl, data, len);

	if (bytes <= 0) {
		while ((error = ERR_get_error())) {
			ERR_error_string_n(error, buf, sizeof(buf));
			warning(("SSL_read error: %s", buf));
		}
		return 0;
	}

	return bytes;
}
