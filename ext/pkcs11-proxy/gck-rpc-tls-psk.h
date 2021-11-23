/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
#ifndef GCKRPC_TLS_PSK_H_
#define GCKRPC_TLS_PSK_H_

#include "openssl/bio.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#if OPENSSL_VERSION_NUMBER < 0x10000000
# error "OpenSSL version >= 1.0.0 required"
#endif

enum gck_rpc_tls_psk_caller {
	GCK_RPC_TLS_PSK_CLIENT,
	GCK_RPC_TLS_PSK_SERVER
};

typedef struct {
	int initialized;
	SSL_CTX *ssl_ctx;
	BIO *bio;
	SSL *ssl;
	enum gck_rpc_tls_psk_caller type;
} GckRpcTlsPskState;


int gck_rpc_init_tls_psk(GckRpcTlsPskState *state, const char *key_filename,
			 const char *identity, enum gck_rpc_tls_psk_caller caller);
int gck_rpc_start_tls(GckRpcTlsPskState *state, int sock);

int gck_rpc_tls_write_all(GckRpcTlsPskState *state, void *data, unsigned int len);
int gck_rpc_tls_read_all(GckRpcTlsPskState *state, void *data, unsigned int len);

void gck_rpc_close_tls(GckRpcTlsPskState *state);

#endif /* GCKRPC_TLS_PSK_H_ */
