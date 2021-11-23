/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gck-rpc-dispatch.h - receiver of our PKCS#11 protocol.

   Copyright (C) 2008, Stef Walter

   The Gnome Keyring Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Keyring Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Stef Walter <stef@memberwebs.com>
*/

#include "config.h"

#include "gck-rpc-layer.h"
#include "gck-rpc-private.h"
#include "gck-rpc-tls-psk.h"

#include "pkcs11/pkcs11.h"
#include "pkcs11/pkcs11g.h"
#include "pkcs11/pkcs11i.h"

#include <sys/types.h>
#include <sys/param.h>
#ifdef __MINGW32__
# include <winsock2.h>
#else
# include <sys/socket.h>
# include <sys/un.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/types.h>
# include <netdb.h>
#endif
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <syslog.h>

#ifdef SECCOMP
#include <seccomp.h>
//#include "seccomp-bpf.h"
#ifdef DEBUG_SECCOMP
# include "syscall-reporter.h"
#endif /* DEBUG_SECCOMP */
#include <fcntl.h> /* for seccomp init */
#endif /* SECCOMP */
#include <sys/mman.h>

/* Where we dispatch the calls to */
static CK_FUNCTION_LIST_PTR pkcs11_module = NULL;

/* The error returned on protocol failures */
#define PARSE_ERROR CKR_DEVICE_ERROR
#define PREP_ERROR  CKR_DEVICE_MEMORY

typedef struct {
	CK_SESSION_HANDLE id;
	CK_SLOT_ID slot;
} SessionState;

typedef struct _CallState {
	GckRpcMessage *req;
	GckRpcMessage *resp;
	void *allocated;
	uint64_t appid;
	int call;
	int sock;
        int (*read)(void *cs, unsigned char *,size_t);
        int (*write)(void *cs, unsigned char *,size_t);
	struct sockaddr_storage addr;
	socklen_t addrlen;
	/* XXX Maybe sessions should be a linked list instead, to remove the hard
	 * upper limit and reduce typical memory use.
	 */
	SessionState sessions[PKCS11PROXY_MAX_SESSION_COUNT];
	GckRpcTlsPskState *tls;
} CallState;

typedef struct _DispatchState {
	struct _DispatchState *next;
	pthread_t thread;
	CallState cs;
} DispatchState;

/* A linked list of dispatcher threads */
static DispatchState *pkcs11_dispatchers = NULL;

/* A mutex to protect the dispatcher list */
static pthread_mutex_t pkcs11_dispatchers_mutex = PTHREAD_MUTEX_INITIALIZER;

/* To be able to call C_Finalize from call_uninit. */
static CK_RV rpc_C_Finalize(CallState *);

static int _install_dispatch_syscall_filter(int use_tls);

/* -----------------------------------------------------------------------------
 * LOGGING and DEBUGGING
 */
#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 1
#endif
#if DEBUG_OUTPUT
#define debug(x) gck_rpc_debug x
#else
#define debug(x)
#endif

#define warning(x) gck_rpc_warn x

#define return_val_if_fail(x, v) \
	if (!(x)) { rpc_warn ("'%s' not true at %s", #x, __func__); return v; }

void gck_rpc_log(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
#if DEBUG_OUTPUT
	vfprintf(stderr, msg, ap);
	fprintf(stderr, "\n");
#else
        vsyslog(LOG_INFO,msg,ap);        
#endif
	va_end(ap);
}

/* -------------------------------------------------------------------------------
 * CALL STRUCTURES
 */

static int call_init(CallState * cs)
{
	assert(cs);

	cs->req = gck_rpc_message_new((EggBufferAllocator) realloc);
	cs->resp = gck_rpc_message_new((EggBufferAllocator) realloc);
	if (!cs->req || !cs->resp) {
		gck_rpc_message_free(cs->req);
		gck_rpc_message_free(cs->resp);
		return 0;
	}

	cs->allocated = NULL;
	return 1;
}

static void *call_alloc(CallState * cs, size_t length)
{
	void **data;

	assert(cs);

	if (length > 0x7fffffff)
		return NULL;

	data = malloc(sizeof(void *) + length);
	if (!data)
		return NULL;

	/* Munch up the memory to help catch bugs */
	memset(data, 0xff, sizeof(void *) + length);

	/* Store pointer to next allocated block at beginning */
	*data = cs->allocated;
	cs->allocated = data;

	/* Data starts after first pointer */
	return (void *)(data + 1);
}

static void call_reset(CallState * cs)
{
	void *allocated;
	void **data;

	assert(cs);

	allocated = cs->allocated;
	while (allocated) {
		data = (void **)allocated;

		/* Pointer to the next allocation */
		allocated = *data;
		free(data);
	}

	cs->allocated = NULL;
	gck_rpc_message_reset(cs->req);
	gck_rpc_message_reset(cs->resp);
}

static void call_uninit(CallState * cs)
{
	assert(cs);

	/* Close any open sessions. Without this, the application won't be able
	 * to reconnect (possibly after a crash).
	 */
	if (cs->req)
		rpc_C_Finalize(cs);

	call_reset(cs);

	gck_rpc_message_free(cs->req);
	gck_rpc_message_free(cs->resp);
}

/* -------------------------------------------------------------------
 * PROTOCOL CODE
 */

static CK_RV
proto_read_byte_buffer(CallState * cs, CK_BYTE_PTR * buffer,
		       CK_ULONG_PTR * n_buffer)
{
	GckRpcMessage *msg;
	uint8_t flags;
	uint32_t length;

	assert(cs);
	assert(buffer);
	assert(n_buffer);

	msg = cs->req;

	/* Check that we're supposed to be reading this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "fy"));

	if (!egg_buffer_get_byte
	    (&msg->buffer, msg->parsed, &msg->parsed, &flags))
		return PARSE_ERROR;

	/* The number of ulongs there's room for on the other end */
	if (!egg_buffer_get_uint32
	    (&msg->buffer, msg->parsed, &msg->parsed, &length))
		return PARSE_ERROR;


	**n_buffer = length;
	*buffer = NULL_PTR;

	if ((flags & GCK_RPC_BYTE_BUFFER_NULL_COUNT))
		*n_buffer = NULL_PTR;

	if (! (flags & GCK_RPC_BYTE_BUFFER_NULL_DATA)) {
		*buffer = call_alloc(cs, length * sizeof(CK_BYTE));
		if (!*buffer)
			return CKR_DEVICE_MEMORY;
	}

	return CKR_OK;
}

static CK_RV
proto_read_byte_array(CallState * cs, CK_BYTE_PTR * array, CK_ULONG * n_array)
{
	GckRpcMessage *msg;
	const unsigned char *data;
	unsigned char valid;
	size_t n_data;

	assert(cs);

	msg = cs->req;

	/* Check that we're supposed to have this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "ay"));

	/* Read out the byte which says whether data is present or not */
	if (!egg_buffer_get_byte
	    (&msg->buffer, msg->parsed, &msg->parsed, &valid))
		return PARSE_ERROR;

	if (!valid) {
		uint32_t n_size;
		/* No array, no data, just length */
		if (!egg_buffer_get_uint32
		    (&msg->buffer, msg->parsed, &msg->parsed, &n_size))
			return PARSE_ERROR;
		*n_array = (size_t) n_size;
		*array = NULL;
		return CKR_OK;
	}

	/* Point our arguments into the buffer */
	if (!egg_buffer_get_byte_array(&msg->buffer, msg->parsed, &msg->parsed,
				       &data, &n_data))
		return PARSE_ERROR;

	*array = (CK_BYTE_PTR) data;
	*n_array = n_data;
	return CKR_OK;
}

static CK_RV
proto_write_byte_array(CallState * cs, CK_BYTE_PTR array, CK_ULONG_PTR len,
		       CK_RV ret)
{
	assert(cs);

	/*
	 * When returning an byte array, in many cases we need to pass
	 * an invalid array along with a length, which signifies CKR_BUFFER_TOO_SMALL.
	 */

	switch (ret) {
	case CKR_BUFFER_TOO_SMALL:
		array = NULL;
		/* fall through */
	case CKR_OK:
		break;

		/* Pass all other errors straight through */
	default:
		return ret;
	};

	if (!gck_rpc_message_write_byte_array(cs->resp, array, len ? *len : 0))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV
proto_read_ulong_buffer(CallState * cs, CK_ULONG_PTR * buffer,
			CK_ULONG * n_buffer)
{
	GckRpcMessage *msg;
	uint32_t length;

	assert(cs);
	assert(buffer);
	assert(n_buffer);

	msg = cs->req;

	/* Check that we're supposed to be reading this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "fu"));

	/* The number of ulongs there's room for on the other end */
	if (!egg_buffer_get_uint32
	    (&msg->buffer, msg->parsed, &msg->parsed, &length))
		return PARSE_ERROR;

	*n_buffer = length;
	*buffer = NULL;

	/* If set to zero, then they just want the length */
	if (!length)
		return CKR_OK;

	*buffer = call_alloc(cs, length * sizeof(CK_ULONG));
	if (!*buffer)
		return CKR_DEVICE_MEMORY;

	return CKR_OK;
}

static CK_RV
proto_write_ulong_array(CallState * cs, CK_ULONG_PTR array, CK_ULONG len,
			CK_RV ret)
{
	assert(cs);

	/*
	 * When returning an ulong array, in many cases we need to pass
	 * an invalid array along with a length, which signifies CKR_BUFFER_TOO_SMALL.
	 */

	switch (ret) {
	case CKR_BUFFER_TOO_SMALL:
		array = NULL;
		/* fall through */
	case CKR_OK:
		break;

		/* Pass all other errors straight through */
	default:
		return ret;
	};

	if (!gck_rpc_message_write_ulong_array(cs->resp, array, len))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV
proto_read_attribute_buffer(CallState * cs, CK_ATTRIBUTE_PTR * result,
			    CK_ULONG * n_result)
{
	CK_ATTRIBUTE_PTR attrs;
	GckRpcMessage *msg;
	uint32_t n_attrs, i;
	uint32_t value;

	assert(cs);
	assert(result);
	assert(n_result);

	msg = cs->req;

	/* Make sure this is in the rigth order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "fA"));

	/* Read the number of attributes */
	if (!egg_buffer_get_uint32
	    (&msg->buffer, msg->parsed, &msg->parsed, &n_attrs))
		return PARSE_ERROR;

	/* Allocate memory for the attribute structures */
	attrs = call_alloc(cs, n_attrs * sizeof(CK_ATTRIBUTE));
	if (!attrs)
		return CKR_DEVICE_MEMORY;

	/* Now go through and fill in each one */
	for (i = 0; i < n_attrs; ++i) {

		/* The attribute type */
		if (!egg_buffer_get_uint32
		    (&msg->buffer, msg->parsed, &msg->parsed, &value))
			return PARSE_ERROR;

		attrs[i].type = value;

		/* The number of bytes to allocate */
		if (!egg_buffer_get_uint32
		    (&msg->buffer, msg->parsed, &msg->parsed, &value))
			return PARSE_ERROR;

		if (value == 0) {
			attrs[i].pValue = NULL;
			attrs[i].ulValueLen = 0;
		} else {
			attrs[i].pValue = call_alloc(cs, value);
			if (!attrs[i].pValue)
				return CKR_DEVICE_MEMORY;
			attrs[i].ulValueLen = value;
		}
	}

	*result = attrs;
	*n_result = n_attrs;
	return CKR_OK;
}

static CK_RV
proto_read_attribute_array(CallState * cs, CK_ATTRIBUTE_PTR * result,
			   CK_ULONG * n_result)
{
	CK_ATTRIBUTE_PTR attrs;
	const unsigned char *data;
	unsigned char valid;
	GckRpcMessage *msg;
	uint32_t n_attrs, i;
	uint32_t value;
	size_t n_data;

	assert(cs);
	assert(result);
	assert(n_result);

	msg = cs->req;

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "aA"));

	/* Read the number of attributes */
	if (!egg_buffer_get_uint32
	    (&msg->buffer, msg->parsed, &msg->parsed, &n_attrs))
		return PARSE_ERROR;

	if (! n_attrs) {
		/* If there are no attributes, it makes most sense to make result
		 * a NULL pointer. What use could one have of a potentially dangling
		 * pointer anyways?
		 */
		*result = NULL_PTR;
		*n_result = n_attrs;
		return CKR_OK;
	}

	/* Allocate memory for the attribute structures */
	attrs = call_alloc(cs, n_attrs * sizeof(CK_ATTRIBUTE));
	if (!attrs)
		return CKR_DEVICE_MEMORY;

	/* Now go through and fill in each one */
	for (i = 0; i < n_attrs; ++i) {

		/* The attribute type */
		if (!egg_buffer_get_uint32
		    (&msg->buffer, msg->parsed, &msg->parsed, &value))
			return PARSE_ERROR;

		attrs[i].type = value;

		/* Whether this one is valid or not */
		if (!egg_buffer_get_byte
		    (&msg->buffer, msg->parsed, &msg->parsed, &valid))
			return PARSE_ERROR;

		if (valid) {
			if (!egg_buffer_get_uint32
			    (&msg->buffer, msg->parsed, &msg->parsed, &value))
				return PARSE_ERROR;
			if (!egg_buffer_get_byte_array
			    (&msg->buffer, msg->parsed, &msg->parsed, &data,
			     &n_data))
				return PARSE_ERROR;

			if (data != NULL && n_data != value) {
				gck_rpc_warn
				    ("attribute length and data do not match");
				return PARSE_ERROR;
			}

			CK_ULONG a;

			if (value == sizeof (uint64_t) &&
			    value != sizeof (CK_ULONG) &&
			    gck_rpc_has_ulong_parameter(attrs[i].type)) {

				value = sizeof (CK_ULONG);
				a = *(uint64_t *)data;
				*(CK_ULONG *)data = a;
			}
			attrs[i].pValue = (CK_VOID_PTR) data;
			attrs[i].ulValueLen = value;
		} else {
			attrs[i].pValue = NULL;
			attrs[i].ulValueLen = -1;
		}
	}

	*result = attrs;
	*n_result = n_attrs;
	return CKR_OK;
}

static CK_RV
proto_write_attribute_array(CallState * cs, CK_ATTRIBUTE_PTR array,
			    CK_ULONG len, CK_RV ret)
{
	assert(cs);

	/*
	 * When returning an attribute array, certain errors aren't
	 * actually real errors, these are passed through to the other
	 * side along with the attribute array.
	 */

	switch (ret) {
	case CKR_ATTRIBUTE_SENSITIVE:
	case CKR_ATTRIBUTE_TYPE_INVALID:
	case CKR_BUFFER_TOO_SMALL:
	case CKR_OK:
		break;

		/* Pass all other errors straight through */
	default:
		return ret;
	};

	if (!gck_rpc_message_write_attribute_array(cs->resp, array, len) ||
	    !gck_rpc_message_write_ulong(cs->resp, ret))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV proto_read_space_string(CallState * cs, CK_UTF8CHAR_PTR * val, CK_ULONG length)
{
	GckRpcMessage *msg;
	const unsigned char *data;
	size_t n_data;

	assert(cs);
	assert(val);

	msg = cs->req;

	/* Check that we're supposed to have this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "s"));

	if (!egg_buffer_get_byte_array
	    (&msg->buffer, msg->parsed, &msg->parsed, &data, &n_data))
		return PARSE_ERROR;

	/* Allocate a block of memory for it. */
	*val = call_alloc(cs, n_data);
	if (!*val)
		return CKR_DEVICE_MEMORY;

	memcpy(*val, data, n_data);

	return CKR_OK;
}

static CK_RV proto_read_mechanism(CallState * cs, CK_MECHANISM_PTR mech)
{
	GckRpcMessage *msg;
	const unsigned char *data;
	uint32_t value;
	size_t n_data;

	assert(cs);
	assert(mech);

	msg = cs->req;

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "M"));

	/* The mechanism type */
	if (!egg_buffer_get_uint32
	    (&msg->buffer, msg->parsed, &msg->parsed, &value))
		return PARSE_ERROR;

	/* The mechanism data */
	if (!egg_buffer_get_byte_array
	    (&msg->buffer, msg->parsed, &msg->parsed, &data, &n_data))
		return PARSE_ERROR;

	mech->mechanism = value;
	mech->pParameter = (CK_VOID_PTR) data;
	mech->ulParameterLen = n_data;
	return CKR_OK;
}

static CK_RV proto_write_info(CallState * cs, CK_INFO_PTR info)
{
	GckRpcMessage *msg;

	assert(cs);
	assert(info);

	msg = cs->resp;

	if (!gck_rpc_message_write_version(msg, &info->cryptokiVersion) ||
	    !gck_rpc_message_write_space_string(msg, info->manufacturerID, 32)
	    || !gck_rpc_message_write_ulong(msg, info->flags)
	    || !gck_rpc_message_write_space_string(msg,
						   info->libraryDescription, 32)
	    || !gck_rpc_message_write_version(msg, &info->libraryVersion))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV proto_write_slot_info(CallState * cs, CK_SLOT_INFO_PTR info)
{
	GckRpcMessage *msg;

	assert(cs);
	assert(info);

	msg = cs->resp;

	if (!gck_rpc_message_write_space_string(msg, info->slotDescription, 64)
	    || !gck_rpc_message_write_space_string(msg, info->manufacturerID,
						   32)
	    || !gck_rpc_message_write_ulong(msg, info->flags)
	    || !gck_rpc_message_write_version(msg, &info->hardwareVersion)
	    || !gck_rpc_message_write_version(msg, &info->firmwareVersion))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV proto_write_token_info(CallState * cs, CK_TOKEN_INFO_PTR info)
{
	GckRpcMessage *msg;

	assert(cs);
	assert(info);

	msg = cs->resp;

	if (!gck_rpc_message_write_space_string(msg, info->label, 32) ||
	    !gck_rpc_message_write_space_string(msg, info->manufacturerID, 32)
	    || !gck_rpc_message_write_space_string(msg, info->model, 16)
	    || !gck_rpc_message_write_space_string(msg, info->serialNumber, 16)
	    || !gck_rpc_message_write_ulong(msg, info->flags)
	    || !gck_rpc_message_write_ulong(msg, info->ulMaxSessionCount)
	    || !gck_rpc_message_write_ulong(msg, info->ulSessionCount)
	    || !gck_rpc_message_write_ulong(msg, info->ulMaxRwSessionCount)
	    || !gck_rpc_message_write_ulong(msg, info->ulRwSessionCount)
	    || !gck_rpc_message_write_ulong(msg, info->ulMaxPinLen)
	    || !gck_rpc_message_write_ulong(msg, info->ulMinPinLen)
	    || !gck_rpc_message_write_ulong(msg, info->ulTotalPublicMemory)
	    || !gck_rpc_message_write_ulong(msg, info->ulFreePublicMemory)
	    || !gck_rpc_message_write_ulong(msg, info->ulTotalPrivateMemory)
	    || !gck_rpc_message_write_ulong(msg, info->ulFreePrivateMemory)
	    || !gck_rpc_message_write_version(msg, &info->hardwareVersion)
	    || !gck_rpc_message_write_version(msg, &info->firmwareVersion)
	    || !gck_rpc_message_write_space_string(msg, info->utcTime, 16))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV
proto_write_mechanism_info(CallState * cs, CK_MECHANISM_INFO_PTR info)
{
	GckRpcMessage *msg;

	assert(cs);
	assert(info);

	msg = cs->resp;

	if (!gck_rpc_message_write_ulong(msg, info->ulMinKeySize) ||
	    !gck_rpc_message_write_ulong(msg, info->ulMaxKeySize) ||
	    !gck_rpc_message_write_ulong(msg, info->flags))
		return PREP_ERROR;

	return CKR_OK;
}

static CK_RV proto_write_session_info(CallState * cs, CK_SESSION_INFO_PTR info)
{
	GckRpcMessage *msg;

	assert(cs);
	assert(info);

	msg = cs->resp;

	if (!gck_rpc_message_write_ulong(msg, info->slotID) ||
	    !gck_rpc_message_write_ulong(msg, info->state) ||
	    !gck_rpc_message_write_ulong(msg, info->flags) ||
	    !gck_rpc_message_write_ulong(msg, info->ulDeviceError))
		return PREP_ERROR;

	return CKR_OK;
}

/* -------------------------------------------------------------------
 * CALL MACROS
 */

#define DECLARE_CK_ULONG_PTR(ck_ulong_ptr_name) \
	CK_ULONG ck_ulong_ptr_name ## _v ; \
	CK_ULONG_PTR ck_ulong_ptr_name ; \
	ck_ulong_ptr_name ## _v = 0; \
	ck_ulong_ptr_name = &ck_ulong_ptr_name ## _v ;

#define BEGIN_CALL(call_id) \
	debug ((#call_id ": enter")); \
	assert (cs); \
	assert (pkcs11_module); \
	{  \
		CK_ ## call_id _func = pkcs11_module-> call_id; \
		CK_RV _ret = CKR_OK; \
		if (!_func) { _ret = CKR_GENERAL_ERROR; goto _cleanup; }

#define PROCESS_CALL(args)\
	assert (gck_rpc_message_is_verified (cs->req)); \
	_ret = _func args

#define END_CALL \
	_cleanup: \
		debug (("ret: 0x%x", _ret)); \
		return _ret; \
	}

#define IN_BYTE(val) \
	if (!gck_rpc_message_read_byte (cs->req, &val)) \
		{ _ret = PARSE_ERROR; goto _cleanup; }

#define IN_ULONG(val) \
	if (!gck_rpc_message_read_ulong (cs->req, &val)) \
		{ _ret = PARSE_ERROR; goto _cleanup; }

#define IN_SPACE_STRING(val, len)			   \
	_ret = proto_read_space_string (cs, &val, len);	   \
	if (_ret != CKR_OK) goto _cleanup;

#define IN_BYTE_BUFFER(buffer, buffer_len_ptr) \
	_ret = proto_read_byte_buffer (cs, &buffer, &buffer_len_ptr); \
	if (_ret != CKR_OK) goto _cleanup;

#define IN_BYTE_ARRAY(buffer, buffer_len) \
	_ret = proto_read_byte_array (cs, &buffer, &buffer_len); \
	if (_ret != CKR_OK) goto _cleanup;

#define IN_ULONG_BUFFER(buffer, buffer_len) \
	_ret = proto_read_ulong_buffer (cs, &buffer, &buffer_len); \
	if (_ret != CKR_OK) goto _cleanup;

#define IN_ATTRIBUTE_BUFFER(buffer, buffer_len) \
	_ret = proto_read_attribute_buffer (cs, &buffer, &buffer_len); \
	if (_ret != CKR_OK) goto _cleanup;

#define IN_ATTRIBUTE_ARRAY(attrs, n_attrs) \
	_ret = proto_read_attribute_array (cs, &attrs, &n_attrs); \
	if (_ret != CKR_OK) goto _cleanup;

#define IN_MECHANISM(mech) \
	_ret = proto_read_mechanism (cs, &mech); \
	if (_ret != CKR_OK) goto _cleanup;

#define OUT_ULONG(val) \
	if (_ret == CKR_OK && !gck_rpc_message_write_ulong (cs->resp, val)) \
		_ret = PREP_ERROR;

#define OUT_BYTE_ARRAY(array, len_ptr) \
	/* Note how we filter return codes */ \
	_ret = proto_write_byte_array (cs, array, len_ptr, _ret);

#define OUT_ULONG_ARRAY(array, len) \
	/* Note how we filter return codes */ \
	_ret = proto_write_ulong_array (cs, array, len, _ret);

#define OUT_ATTRIBUTE_ARRAY(array, len) \
	/* Note how we filter return codes */ \
	_ret = proto_write_attribute_array (cs, array, len, _ret);

#define OUT_INFO(val) \
	if (_ret == CKR_OK) \
		_ret = proto_write_info (cs, &val);

#define OUT_SLOT_INFO(val) \
	if (_ret == CKR_OK) \
		_ret = proto_write_slot_info (cs, &val);

#define OUT_TOKEN_INFO(val) \
	if (_ret == CKR_OK) \
		_ret = proto_write_token_info (cs, &val);

#define OUT_MECHANISM_INFO(val) \
	if (_ret == CKR_OK) \
		_ret = proto_write_mechanism_info (cs, &val);

#define OUT_SESSION_INFO(val) \
	if (_ret == CKR_OK) \
		_ret = proto_write_session_info (cs, &val);

/* ---------------------------------------------------------------------------
 * DISPATCH SPECIFIC CALLS
 */

static CK_RV rpc_C_Initialize(CallState * cs)
{
	CK_BYTE_PTR handshake;
	CK_ULONG n_handshake;
	CK_RV ret = CKR_OK;

	debug(("C_Initialize: enter"));

	assert(cs);
	assert(pkcs11_module);

	ret = proto_read_byte_array(cs, &handshake, &n_handshake);
	if (ret == CKR_OK) {

		/* Check to make sure the header matches */
		if (n_handshake != GCK_RPC_HANDSHAKE_LEN ||
		    handshake == NULL_PTR ||
		    memcmp(handshake, GCK_RPC_HANDSHAKE, n_handshake) != 0) {
			gck_rpc_warn
			    ("invalid handshake received from connecting module");
			ret = CKR_GENERAL_ERROR;
		}

		assert(gck_rpc_message_is_verified(cs->req));
	}

	/*
	 * We don't actually C_Initialize lower layers. It's assumed
	 * that they'll already be initialzied by the code that loaded us.
	 */

	debug(("ret: %d", ret));
	return ret;
}

static CK_RV rpc_C_Finalize(CallState * cs)
{
	CK_ULONG i;
	CK_RV ret;
	DispatchState *ds, *next;


	debug(("C_Finalize: enter"));

	assert(cs);
	assert(pkcs11_module);

	/*
	 * We don't actually C_Finalize lower layers, since this would finalize
	 * for all appartments, client applications. Anyway this is done by
	 * the code that loaded us.
	 */

	ret = CKR_OK;

	/* Close all sessions that have been opened by this thread, regardless of slot */
	for (i = 0; i < PKCS11PROXY_MAX_SESSION_COUNT; i++) {
		if (cs->sessions[i].id) {
			gck_rpc_log("Closing session %li on position %i", cs->sessions[i].id, i);

			ret = (pkcs11_module->C_CloseSession) (cs->sessions[i].id);
			if (ret != CKR_OK)
				break;
			cs->sessions[i].id = 0;
		}
	}

	/* Make all C_WaitForSlotEvent calls return */
	pthread_mutex_lock(&pkcs11_dispatchers_mutex);
	for (ds = pkcs11_dispatchers; ds; ds = next) {
		CallState *c = &ds->cs;

                next = ds->next;

		if (c->appid != cs->appid)
			continue ;
		if (c->sock == cs->sock)
			continue ;
		if (c->req &&
		    (c->req->call_id == GCK_RPC_CALL_C_WaitForSlotEvent)) {
			gck_rpc_log("Sending interuption signal to %i\n",
                                    c->sock);
			if (c->sock != -1)
				if (shutdown(c->sock, SHUT_RDWR) == 0)
					c->sock = -1;
			//pthread_kill(ds->thread, SIGINT);
		}
	}
	pthread_mutex_unlock(&pkcs11_dispatchers_mutex);

	debug(("ret: %d", ret));
	return ret;
}

static CK_RV rpc_C_GetInfo(CallState * cs)
{
	CK_INFO info;

	BEGIN_CALL(C_GetInfo);
	PROCESS_CALL((&info));
	OUT_INFO(info);
	END_CALL;
}

static CK_RV rpc_C_GetSlotList(CallState * cs)
{
	CK_BBOOL token_present;
	CK_SLOT_ID_PTR slot_list;
	CK_ULONG count;

	BEGIN_CALL(C_GetSlotList);
	IN_BYTE(token_present);
	IN_ULONG_BUFFER(slot_list, count);
	PROCESS_CALL((token_present, slot_list, &count));
	OUT_ULONG_ARRAY(slot_list, count);
	END_CALL;
}

static CK_RV rpc_C_GetSlotInfo(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_SLOT_INFO info;

	/* Slot id becomes appartment so lower layers can tell clients apart. */

	BEGIN_CALL(C_GetSlotInfo);
	IN_ULONG(slot_id);
	PROCESS_CALL((slot_id, &info));
	OUT_SLOT_INFO(info);
	END_CALL;
}

static CK_RV rpc_C_GetTokenInfo(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_TOKEN_INFO info;

	/* Slot id becomes appartment so lower layers can tell clients apart. */

	BEGIN_CALL(C_GetTokenInfo);
	IN_ULONG(slot_id);
	PROCESS_CALL((slot_id, &info));
	OUT_TOKEN_INFO(info);
	END_CALL;
}

static CK_RV rpc_C_GetMechanismList(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_MECHANISM_TYPE_PTR mechanism_list;
	CK_ULONG count;

	/* Slot id becomes appartment so lower layers can tell clients apart. */

	BEGIN_CALL(C_GetMechanismList);
	IN_ULONG(slot_id);
	IN_ULONG_BUFFER(mechanism_list, count);
	PROCESS_CALL((slot_id, mechanism_list, &count));
	OUT_ULONG_ARRAY(mechanism_list, count);
	END_CALL;
}

static CK_RV rpc_C_GetMechanismInfo(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_MECHANISM_TYPE type;
	CK_MECHANISM_INFO info;

	/* Slot id becomes appartment so lower layers can tell clients apart. */

	BEGIN_CALL(C_GetMechanismInfo);
	IN_ULONG(slot_id);
	IN_ULONG(type);
	PROCESS_CALL((slot_id, type, &info));
	OUT_MECHANISM_INFO(info);
	END_CALL;
}

static CK_RV rpc_C_InitToken(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_UTF8CHAR_PTR pin;
	CK_ULONG pin_len;
	CK_UTF8CHAR_PTR label;

	/* Slot id becomes appartment so lower layers can tell clients apart. */

	BEGIN_CALL(C_InitToken);
	IN_ULONG(slot_id);
	IN_BYTE_ARRAY(pin, pin_len);
	IN_SPACE_STRING(label, 32);
	PROCESS_CALL((slot_id, pin, pin_len, label));
	END_CALL;
}

static CK_RV rpc_C_WaitForSlotEvent(CallState * cs)
{
	CK_FLAGS flags;
	CK_SLOT_ID slot_id;

	/* Get slot id from appartment lower layers use. */

	BEGIN_CALL(C_WaitForSlotEvent);
	IN_ULONG(flags);
	PROCESS_CALL((flags, &slot_id, NULL));
	slot_id = CK_GNOME_APPARTMENT_SLOT(slot_id);
	OUT_ULONG(slot_id);
	END_CALL;
}

static CK_RV rpc_C_OpenSession(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_FLAGS flags;
	CK_SESSION_HANDLE session;

	/* Slot id becomes appartment so lower layers can tell clients apart. */

	BEGIN_CALL(C_OpenSession);
	IN_ULONG(slot_id);
	IN_ULONG(flags);
	PROCESS_CALL((slot_id, flags, NULL, NULL, &session));
	if (_ret == CKR_OK) {
		int i;
		/* Remember this thread opened this session. Needed for C_CloseAllSessions. */
		for (i = 0; i < PKCS11PROXY_MAX_SESSION_COUNT; i++) {
			if (! cs->sessions[i].id) {
				cs->sessions[i].id = session;
				cs->sessions[i].slot = slot_id;
				gck_rpc_log("Session %li stored in position %i", session, i);
				break;
			}
		}
		if (i == PKCS11PROXY_MAX_SESSION_COUNT) {
			_ret = CKR_SESSION_COUNT; goto _cleanup;
		}
	}
	OUT_ULONG(session);
	END_CALL;
}

static CK_RV rpc_C_CloseSession(CallState * cs)
{
	CK_SESSION_HANDLE session;

	BEGIN_CALL(C_CloseSession);
	IN_ULONG(session);
	PROCESS_CALL((session));
	if (_ret == CKR_OK) {
		int i;
		/* Remove this session from this threads list */
		for (i = 0; i < PKCS11PROXY_MAX_SESSION_COUNT; i++) {
			if (cs->sessions[i].id == session) {
				gck_rpc_log("Session %li removed from position %i", session, i);
				cs->sessions[i].id = 0;
				break;
			}
		}
		if (i == PKCS11PROXY_MAX_SESSION_COUNT) {
			/* Ignore errors, like with close() */
			gck_rpc_log("C_CloseSession on unknown session");
		}
	}
	END_CALL;
}

static CK_RV rpc_C_CloseAllSessions(CallState * cs)
{
	CK_SLOT_ID slot_id;
	CK_SLOT_INFO slotInfo;
	int i;

	/* Close all sessions that have been opened by this thread. PKCS#11 (v2.2) says
	 * C_CloseAllSessions closes all the sessions opened by one application, leaving
	 * sessions opened by other applications alone even if the sessions share slot.
	 *
	 * Each application on the client side of pkcs11-proxy will mean different thread
	 * on the server side, so we should close all sessions for a slot opened in this
	 * thread.
	 */

	BEGIN_CALL(C_CloseAllSessions);
	IN_ULONG(slot_id);

	/* To emulate real C_CloseAllSessions (well, the SoftHSM one) we check if slot_id is valid. */
	_ret = pkcs11_module->C_GetSlotInfo(slot_id, &slotInfo);
	if (_ret != CKR_OK)
		goto _cleanup;

	for (i = 0; i < PKCS11PROXY_MAX_SESSION_COUNT; i++) {
		if (cs->sessions[i].id && (cs->sessions[i].slot == slot_id)) {
			gck_rpc_log("Closing session %li on position %i with slot %i", cs->sessions[i].id, i, slot_id);

			_ret = (pkcs11_module->C_CloseSession) (cs->sessions[i].id);
			if (_ret == CKR_OK ||
			    _ret == CKR_SESSION_CLOSED ||
			    _ret == CKR_SESSION_HANDLE_INVALID) {
				cs->sessions[i].id = 0;
			}
			if (_ret != CKR_OK)
				goto _cleanup;
		}
	}
	END_CALL;
}

static CK_RV rpc_C_GetFunctionStatus(CallState * cs)
{
	CK_SESSION_HANDLE session;

	BEGIN_CALL(C_GetFunctionStatus);
	IN_ULONG(session);
	PROCESS_CALL((session));
	END_CALL;
}

static CK_RV rpc_C_CancelFunction(CallState * cs)
{
	CK_SESSION_HANDLE session;

	BEGIN_CALL(C_CancelFunction);
	IN_ULONG(session);
	PROCESS_CALL((session));
	END_CALL;
}

static CK_RV rpc_C_GetSessionInfo(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_SESSION_INFO info;

	/* Get slot id from appartment lower layers use. */

	BEGIN_CALL(C_GetSessionInfo);
	IN_ULONG(session);
	PROCESS_CALL((session, &info));
	info.slotID = CK_GNOME_APPARTMENT_SLOT(info.slotID);
	OUT_SESSION_INFO(info);
	END_CALL;
}

static CK_RV rpc_C_InitPIN(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_UTF8CHAR_PTR pin;
	CK_ULONG pin_len;

	BEGIN_CALL(C_InitPIN);
	IN_ULONG(session);
	IN_BYTE_ARRAY(pin, pin_len);
	PROCESS_CALL((session, pin, pin_len));
	END_CALL;
}

static CK_RV rpc_C_SetPIN(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_UTF8CHAR_PTR old_pin;
	CK_ULONG old_len;
	CK_UTF8CHAR_PTR new_pin;
	CK_ULONG new_len;

	BEGIN_CALL(C_SetPIN);
	IN_ULONG(session);
	IN_BYTE_ARRAY(old_pin, old_len);
	IN_BYTE_ARRAY(new_pin, new_len);
	PROCESS_CALL((session, old_pin, old_len, new_pin, new_len));
	END_CALL;
}

static CK_RV rpc_C_GetOperationState(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR operation_state;
	DECLARE_CK_ULONG_PTR(operation_state_len);

	BEGIN_CALL(C_GetOperationState);
	IN_ULONG(session);
	IN_BYTE_BUFFER(operation_state, operation_state_len);
	PROCESS_CALL((session, operation_state, operation_state_len));
	OUT_BYTE_ARRAY(operation_state, operation_state_len);
	END_CALL;
}

static CK_RV rpc_C_SetOperationState(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR operation_state;
	CK_ULONG operation_state_len;
	CK_OBJECT_HANDLE encryption_key;
	CK_OBJECT_HANDLE authentication_key;

	BEGIN_CALL(C_SetOperationState);
	IN_ULONG(session);
	IN_BYTE_ARRAY(operation_state, operation_state_len);
	IN_ULONG(encryption_key);
	IN_ULONG(authentication_key);
	PROCESS_CALL((session, operation_state, operation_state_len,
		      encryption_key, authentication_key));
	END_CALL;
}

static CK_RV rpc_C_Login(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_USER_TYPE user_type;
	CK_UTF8CHAR_PTR pin;
	CK_ULONG pin_len;

	BEGIN_CALL(C_Login);
	IN_ULONG(session);
	IN_ULONG(user_type);
	IN_BYTE_ARRAY(pin, pin_len);
	PROCESS_CALL((session, user_type, pin, pin_len));
	END_CALL;
}

static CK_RV rpc_C_Logout(CallState * cs)
{
	CK_SESSION_HANDLE session;

	BEGIN_CALL(C_Logout);
	IN_ULONG(session);
	PROCESS_CALL((session));
	END_CALL;
}

/* -----------------------------------------------------------------------------
 * OBJECT OPERATIONS
 */

static CK_RV rpc_C_CreateObject(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG count;
	CK_OBJECT_HANDLE new_object;

	BEGIN_CALL(C_CreateObject);
	IN_ULONG(session);
	IN_ATTRIBUTE_ARRAY(template, count);
	PROCESS_CALL((session, template, count, &new_object));
	OUT_ULONG(new_object);
	END_CALL;
}

static CK_RV rpc_C_CopyObject(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE object;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG count;
	CK_OBJECT_HANDLE new_object;

	BEGIN_CALL(C_CopyObject);
	IN_ULONG(session);
	IN_ULONG(object);
	IN_ATTRIBUTE_ARRAY(template, count);
	PROCESS_CALL((session, object, template, count, &new_object));
	OUT_ULONG(new_object);
	END_CALL;
}

static CK_RV rpc_C_DestroyObject(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE object;

	BEGIN_CALL(C_DestroyObject);
	IN_ULONG(session);
	IN_ULONG(object);
	PROCESS_CALL((session, object));
	END_CALL;
}

static CK_RV rpc_C_GetObjectSize(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE object;
	CK_ULONG size;

	BEGIN_CALL(C_GetObjectSize);
	IN_ULONG(session);
	IN_ULONG(object);
	PROCESS_CALL((session, object, &size));
	OUT_ULONG(size);
	END_CALL;
}

static CK_RV rpc_C_GetAttributeValue(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE object;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG count;

	BEGIN_CALL(C_GetAttributeValue);
	IN_ULONG(session);
	IN_ULONG(object);
	IN_ATTRIBUTE_BUFFER(template, count);
	PROCESS_CALL((session, object, template, count));
	OUT_ATTRIBUTE_ARRAY(template, count);
	END_CALL;
}

static CK_RV rpc_C_SetAttributeValue(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE object;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG count;

	BEGIN_CALL(C_SetAttributeValue);
	IN_ULONG(session);
	IN_ULONG(object);
	IN_ATTRIBUTE_ARRAY(template, count);
	PROCESS_CALL((session, object, template, count));
	END_CALL;
}

static CK_RV rpc_C_FindObjectsInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG count;

	BEGIN_CALL(C_FindObjectsInit);
	IN_ULONG(session);
	IN_ATTRIBUTE_ARRAY(template, count);
	PROCESS_CALL((session, template, count));
	END_CALL;
}

static CK_RV rpc_C_FindObjects(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE_PTR objects;
	CK_ULONG max_object_count;
	CK_ULONG object_count;

	BEGIN_CALL(C_FindObjects);
	IN_ULONG(session);
	IN_ULONG_BUFFER(objects, max_object_count);
	PROCESS_CALL((session, objects, max_object_count, &object_count));
	OUT_ULONG_ARRAY(objects, object_count);
	END_CALL;
}

static CK_RV rpc_C_FindObjectsFinal(CallState * cs)
{
	CK_SESSION_HANDLE session;

	BEGIN_CALL(C_FindObjectsFinal);
	IN_ULONG(session);
	PROCESS_CALL((session));
	END_CALL;
}

static CK_RV rpc_C_EncryptInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_EncryptInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(key);
	PROCESS_CALL((session, &mechanism, key));
	END_CALL;

}

static CK_RV rpc_C_Encrypt(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR data;
	CK_ULONG data_len;
	CK_BYTE_PTR encrypted_data;
	DECLARE_CK_ULONG_PTR(encrypted_data_len);

	BEGIN_CALL(C_Encrypt);
	IN_ULONG(session);
	IN_BYTE_ARRAY(data, data_len);
	IN_BYTE_BUFFER(encrypted_data, encrypted_data_len);
	PROCESS_CALL((session, data, data_len, encrypted_data,
		      encrypted_data_len));
	OUT_BYTE_ARRAY(encrypted_data, encrypted_data_len);
	END_CALL;
}

static CK_RV rpc_C_EncryptUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;
	CK_BYTE_PTR encrypted_part;
	DECLARE_CK_ULONG_PTR(encrypted_part_len);

	BEGIN_CALL(C_EncryptUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	IN_BYTE_BUFFER(encrypted_part, encrypted_part_len);
	PROCESS_CALL((session, part, part_len, encrypted_part,
		      encrypted_part_len));
	OUT_BYTE_ARRAY(encrypted_part, encrypted_part_len);
	END_CALL;
}

static CK_RV rpc_C_EncryptFinal(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR last_encrypted_part;
	DECLARE_CK_ULONG_PTR(last_encrypted_part_len);

	BEGIN_CALL(C_EncryptFinal);
	IN_ULONG(session);
	IN_BYTE_BUFFER(last_encrypted_part, last_encrypted_part_len);
	PROCESS_CALL((session, last_encrypted_part, last_encrypted_part_len));
	OUT_BYTE_ARRAY(last_encrypted_part, last_encrypted_part_len);
	END_CALL;
}

static CK_RV rpc_C_DecryptInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_DecryptInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(key);
	PROCESS_CALL((session, &mechanism, key));
	END_CALL;
}

static CK_RV rpc_C_Decrypt(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR encrypted_data;
	CK_ULONG encrypted_data_len;
	CK_BYTE_PTR data;
	DECLARE_CK_ULONG_PTR(data_len);

	BEGIN_CALL(C_Decrypt);
	IN_ULONG(session);
	IN_BYTE_ARRAY(encrypted_data, encrypted_data_len);
	IN_BYTE_BUFFER(data, data_len);
	PROCESS_CALL((session, encrypted_data, encrypted_data_len, data,
		      data_len));
	OUT_BYTE_ARRAY(data, data_len);
	END_CALL;
}

static CK_RV rpc_C_DecryptUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR encrypted_part;
	CK_ULONG encrypted_part_len;
	CK_BYTE_PTR part;
	DECLARE_CK_ULONG_PTR(part_len);

	BEGIN_CALL(C_DecryptUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(encrypted_part, encrypted_part_len);
	IN_BYTE_BUFFER(part, part_len);
	PROCESS_CALL((session, encrypted_part, encrypted_part_len, part,
		      part_len));
	OUT_BYTE_ARRAY(part, part_len);
	END_CALL;
}

static CK_RV rpc_C_DecryptFinal(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR last_part;
	DECLARE_CK_ULONG_PTR(last_part_len);

	BEGIN_CALL(C_DecryptFinal);
	IN_ULONG(session);
	IN_BYTE_BUFFER(last_part, last_part_len);
	PROCESS_CALL((session, last_part, last_part_len));
	OUT_BYTE_ARRAY(last_part, last_part_len);
	END_CALL;
}

static CK_RV rpc_C_DigestInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;

	BEGIN_CALL(C_DigestInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	PROCESS_CALL((session, &mechanism));
	END_CALL;
}

static CK_RV rpc_C_Digest(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR data;
	CK_ULONG data_len;
	CK_BYTE_PTR digest;
	DECLARE_CK_ULONG_PTR(digest_len);

	BEGIN_CALL(C_Digest);
	IN_ULONG(session);
	IN_BYTE_ARRAY(data, data_len);
	IN_BYTE_BUFFER(digest, digest_len);
	PROCESS_CALL((session, data, data_len, digest, digest_len));
	OUT_BYTE_ARRAY(digest, digest_len);
	END_CALL;
}

static CK_RV rpc_C_DigestUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;

	BEGIN_CALL(C_DigestUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	PROCESS_CALL((session, part, part_len));
	END_CALL;
}

static CK_RV rpc_C_DigestKey(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_DigestKey);
	IN_ULONG(session);
	IN_ULONG(key);
	PROCESS_CALL((session, key));
	END_CALL;
}

static CK_RV rpc_C_DigestFinal(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR digest;
	DECLARE_CK_ULONG_PTR(digest_len);

	BEGIN_CALL(C_DigestFinal);
	IN_ULONG(session);
	IN_BYTE_BUFFER(digest, digest_len);
	PROCESS_CALL((session, digest, digest_len));
	OUT_BYTE_ARRAY(digest, digest_len);
	END_CALL;
}

static CK_RV rpc_C_SignInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_SignInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(key);
	PROCESS_CALL((session, &mechanism, key));
	END_CALL;
}

static CK_RV rpc_C_Sign(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;
	CK_BYTE_PTR signature;
	DECLARE_CK_ULONG_PTR(signature_len);

	BEGIN_CALL(C_Sign);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	IN_BYTE_BUFFER(signature, signature_len);
	PROCESS_CALL((session, part, part_len, signature, signature_len));
	OUT_BYTE_ARRAY(signature, signature_len);
	END_CALL;

}

static CK_RV rpc_C_SignUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;

	BEGIN_CALL(C_SignUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	PROCESS_CALL((session, part, part_len));
	END_CALL;
}

static CK_RV rpc_C_SignFinal(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR signature;
	DECLARE_CK_ULONG_PTR(signature_len);

	BEGIN_CALL(C_SignFinal);
	IN_ULONG(session);
	IN_BYTE_BUFFER(signature, signature_len);
	PROCESS_CALL((session, signature, signature_len));
	OUT_BYTE_ARRAY(signature, signature_len);
	END_CALL;
}

static CK_RV rpc_C_SignRecoverInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_SignRecoverInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(key);
	PROCESS_CALL((session, &mechanism, key));
	END_CALL;
}

static CK_RV rpc_C_SignRecover(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR data;
	CK_ULONG data_len;
	CK_BYTE_PTR signature;
	DECLARE_CK_ULONG_PTR(signature_len);

	BEGIN_CALL(C_SignRecover);
	IN_ULONG(session);
	IN_BYTE_ARRAY(data, data_len);
	IN_BYTE_BUFFER(signature, signature_len);
	PROCESS_CALL((session, data, data_len, signature, signature_len));
	OUT_BYTE_ARRAY(signature, signature_len);
	END_CALL;
}

static CK_RV rpc_C_VerifyInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_VerifyInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(key);
	PROCESS_CALL((session, &mechanism, key));
	END_CALL;
}

static CK_RV rpc_C_Verify(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR data;
	CK_ULONG data_len;
	CK_BYTE_PTR signature;
	CK_ULONG signature_len;

	BEGIN_CALL(C_Verify);
	IN_ULONG(session);
	IN_BYTE_ARRAY(data, data_len);
	IN_BYTE_ARRAY(signature, signature_len);
	PROCESS_CALL((session, data, data_len, signature, signature_len));
	END_CALL;
}

static CK_RV rpc_C_VerifyUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;

	BEGIN_CALL(C_VerifyUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	PROCESS_CALL((session, part, part_len));
	END_CALL;
}

static CK_RV rpc_C_VerifyFinal(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR signature;
	CK_ULONG signature_len;

	BEGIN_CALL(C_VerifyFinal);
	IN_ULONG(session);
	IN_BYTE_ARRAY(signature, signature_len);
	PROCESS_CALL((session, signature, signature_len));
	END_CALL;
}

static CK_RV rpc_C_VerifyRecoverInit(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_VerifyRecoverInit);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(key);
	PROCESS_CALL((session, &mechanism, key));
	END_CALL;
}

static CK_RV rpc_C_VerifyRecover(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR signature;
	CK_ULONG signature_len;
	CK_BYTE_PTR data;
	DECLARE_CK_ULONG_PTR(data_len);

	BEGIN_CALL(C_VerifyRecover);
	IN_ULONG(session);
	IN_BYTE_ARRAY(signature, signature_len);
	IN_BYTE_BUFFER(data, data_len);
	PROCESS_CALL((session, signature, signature_len, data, data_len));
	OUT_BYTE_ARRAY(data, data_len);
	END_CALL;
}

static CK_RV rpc_C_DigestEncryptUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;
	CK_BYTE_PTR encrypted_part;
	DECLARE_CK_ULONG_PTR(encrypted_part_len);

	BEGIN_CALL(C_DigestEncryptUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	IN_BYTE_BUFFER(encrypted_part, encrypted_part_len);
	PROCESS_CALL((session, part, part_len, encrypted_part,
		      encrypted_part_len));
	OUT_BYTE_ARRAY(encrypted_part, encrypted_part_len);
	END_CALL;
}

static CK_RV rpc_C_DecryptDigestUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR encrypted_part;
	CK_ULONG encrypted_part_len;
	CK_BYTE_PTR part;
	DECLARE_CK_ULONG_PTR(part_len);

	BEGIN_CALL(C_DecryptDigestUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(encrypted_part, encrypted_part_len);
	IN_BYTE_BUFFER(part, part_len);
	PROCESS_CALL((session, encrypted_part, encrypted_part_len, part,
		      part_len));
	OUT_BYTE_ARRAY(part, part_len);
	END_CALL;
}

static CK_RV rpc_C_SignEncryptUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR part;
	CK_ULONG part_len;
	CK_BYTE_PTR encrypted_part;
	DECLARE_CK_ULONG_PTR(encrypted_part_len);

	BEGIN_CALL(C_SignEncryptUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(part, part_len);
	IN_BYTE_BUFFER(encrypted_part, encrypted_part_len);
	PROCESS_CALL((session, part, part_len, encrypted_part,
		      encrypted_part_len));
	OUT_BYTE_ARRAY(encrypted_part, encrypted_part_len);
	END_CALL;
}

static CK_RV rpc_C_DecryptVerifyUpdate(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR encrypted_part;
	CK_ULONG encrypted_part_len;
	CK_BYTE_PTR part;
	DECLARE_CK_ULONG_PTR(part_len);

	BEGIN_CALL(C_DecryptVerifyUpdate);
	IN_ULONG(session);
	IN_BYTE_ARRAY(encrypted_part, encrypted_part_len);
	IN_BYTE_BUFFER(part, part_len);
	PROCESS_CALL((session, encrypted_part, encrypted_part_len, part,
		      part_len));
	OUT_BYTE_ARRAY(part, part_len);
	END_CALL;
}

/* -----------------------------------------------------------------------------
 * KEY OPERATIONS
 */

static CK_RV rpc_C_GenerateKey(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG count;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_GenerateKey);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ATTRIBUTE_ARRAY(template, count);
	PROCESS_CALL((session, &mechanism, template, count, &key));
	OUT_ULONG(key);
	END_CALL;
}

static CK_RV rpc_C_GenerateKeyPair(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_ATTRIBUTE_PTR public_key_template;
	CK_ULONG public_key_attribute_count;
	CK_ATTRIBUTE_PTR private_key_template;
	CK_ULONG private_key_attribute_count;
	CK_OBJECT_HANDLE public_key;
	CK_OBJECT_HANDLE private_key;

	BEGIN_CALL(C_GenerateKeyPair);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ATTRIBUTE_ARRAY(public_key_template, public_key_attribute_count);
	IN_ATTRIBUTE_ARRAY(private_key_template, private_key_attribute_count);
	PROCESS_CALL((session, &mechanism, public_key_template,
		      public_key_attribute_count, private_key_template,
		      private_key_attribute_count, &public_key, &private_key));
	OUT_ULONG(public_key);
	OUT_ULONG(private_key);
	END_CALL;

}

static CK_RV rpc_C_WrapKey(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE wrapping_key;
	CK_OBJECT_HANDLE key;
	CK_BYTE_PTR wrapped_key;
	DECLARE_CK_ULONG_PTR(wrapped_key_len);

	BEGIN_CALL(C_WrapKey);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(wrapping_key);
	IN_ULONG(key);
	IN_BYTE_BUFFER(wrapped_key, wrapped_key_len);
	PROCESS_CALL((session, &mechanism, wrapping_key, key, wrapped_key,
		      wrapped_key_len));
	OUT_BYTE_ARRAY(wrapped_key, wrapped_key_len);
	END_CALL;
}

static CK_RV rpc_C_UnwrapKey(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE unwrapping_key;
	CK_BYTE_PTR wrapped_key;
	CK_ULONG wrapped_key_len;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG attribute_count;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_UnwrapKey);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(unwrapping_key);
	IN_BYTE_ARRAY(wrapped_key, wrapped_key_len);
	IN_ATTRIBUTE_ARRAY(template, attribute_count);
	PROCESS_CALL((session, &mechanism, unwrapping_key, wrapped_key,
		      wrapped_key_len, template, attribute_count, &key));
	OUT_ULONG(key);
	END_CALL;
}

static CK_RV rpc_C_DeriveKey(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_MECHANISM mechanism;
	CK_OBJECT_HANDLE base_key;
	CK_ATTRIBUTE_PTR template;
	CK_ULONG attribute_count;
	CK_OBJECT_HANDLE key;

	BEGIN_CALL(C_DeriveKey);
	IN_ULONG(session);
	IN_MECHANISM(mechanism);
	IN_ULONG(base_key);
	IN_ATTRIBUTE_ARRAY(template, attribute_count);
	PROCESS_CALL((session, &mechanism, base_key, template, attribute_count,
		      &key));
	OUT_ULONG(key);
	END_CALL;
}

static CK_RV rpc_C_SeedRandom(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR seed;
	CK_ULONG seed_len;

	BEGIN_CALL(C_SeedRandom);
	IN_ULONG(session);
	IN_BYTE_ARRAY(seed, seed_len);
	PROCESS_CALL((session, seed, seed_len));
	END_CALL;
}

static CK_RV rpc_C_GenerateRandom(CallState * cs)
{
	CK_SESSION_HANDLE session;
	CK_BYTE_PTR random_data;
	DECLARE_CK_ULONG_PTR(random_len);

	BEGIN_CALL(C_GenerateRandom);
	IN_ULONG(session);
	IN_BYTE_BUFFER(random_data, random_len);
	if (random_len == NULL_PTR) {
		_ret = PARSE_ERROR; goto _cleanup;
	}
	PROCESS_CALL((session, random_data, *random_len));
	OUT_BYTE_ARRAY(random_data, random_len);
	END_CALL;
}

/* ---------------------------------------------------------------------------
 * DISPATCH THREAD HANDLING
 */

static int dispatch_call(CallState * cs)
{
	GckRpcMessage *req, *resp;
	CK_RV ret = CKR_OK;

	assert(cs);

	req = cs->req;
	resp = cs->resp;

	/* This should have been checked by the parsing code */
	assert(req->call_id > GCK_RPC_CALL_ERROR);
	assert(req->call_id < GCK_RPC_CALL_MAX);

	/* Prepare a response for the function to fill in */
	if (!gck_rpc_message_prep(resp, req->call_id, GCK_RPC_RESPONSE)) {
		gck_rpc_warn("couldn't prepare message");
		return 0;
	}

	switch (req->call_id) {

#define CASE_CALL(name) \
		case GCK_RPC_CALL_##name: \
			ret = rpc_##name (cs); \
			break;
		CASE_CALL(C_Initialize)
		    CASE_CALL(C_Finalize)
		    CASE_CALL(C_GetInfo)
		    CASE_CALL(C_GetSlotList)
		    CASE_CALL(C_GetSlotInfo)
		    CASE_CALL(C_GetTokenInfo)
		    CASE_CALL(C_GetMechanismList)
		    CASE_CALL(C_GetMechanismInfo)
		    CASE_CALL(C_InitToken)
		    CASE_CALL(C_WaitForSlotEvent)
		    CASE_CALL(C_OpenSession)
		    CASE_CALL(C_CloseSession)
		    CASE_CALL(C_CloseAllSessions)
		    CASE_CALL(C_GetFunctionStatus)
		    CASE_CALL(C_CancelFunction)
		    CASE_CALL(C_GetSessionInfo)
		    CASE_CALL(C_InitPIN)
		    CASE_CALL(C_SetPIN)
		    CASE_CALL(C_GetOperationState)
		    CASE_CALL(C_SetOperationState)
		    CASE_CALL(C_Login)
		    CASE_CALL(C_Logout)
		    CASE_CALL(C_CreateObject)
		    CASE_CALL(C_CopyObject)
		    CASE_CALL(C_DestroyObject)
		    CASE_CALL(C_GetObjectSize)
		    CASE_CALL(C_GetAttributeValue)
		    CASE_CALL(C_SetAttributeValue)
		    CASE_CALL(C_FindObjectsInit)
		    CASE_CALL(C_FindObjects)
		    CASE_CALL(C_FindObjectsFinal)
		    CASE_CALL(C_EncryptInit)
		    CASE_CALL(C_Encrypt)
		    CASE_CALL(C_EncryptUpdate)
		    CASE_CALL(C_EncryptFinal)
		    CASE_CALL(C_DecryptInit)
		    CASE_CALL(C_Decrypt)
		    CASE_CALL(C_DecryptUpdate)
		    CASE_CALL(C_DecryptFinal)
		    CASE_CALL(C_DigestInit)
		    CASE_CALL(C_Digest)
		    CASE_CALL(C_DigestUpdate)
		    CASE_CALL(C_DigestKey)
		    CASE_CALL(C_DigestFinal)
		    CASE_CALL(C_SignInit)
		    CASE_CALL(C_Sign)
		    CASE_CALL(C_SignUpdate)
		    CASE_CALL(C_SignFinal)
		    CASE_CALL(C_SignRecoverInit)
		    CASE_CALL(C_SignRecover)
		    CASE_CALL(C_VerifyInit)
		    CASE_CALL(C_Verify)
		    CASE_CALL(C_VerifyUpdate)
		    CASE_CALL(C_VerifyFinal)
		    CASE_CALL(C_VerifyRecoverInit)
		    CASE_CALL(C_VerifyRecover)
		    CASE_CALL(C_DigestEncryptUpdate)
		    CASE_CALL(C_DecryptDigestUpdate)
		    CASE_CALL(C_SignEncryptUpdate)
		    CASE_CALL(C_DecryptVerifyUpdate)
		    CASE_CALL(C_GenerateKey)
		    CASE_CALL(C_GenerateKeyPair)
		    CASE_CALL(C_WrapKey)
		    CASE_CALL(C_UnwrapKey)
		    CASE_CALL(C_DeriveKey)
		    CASE_CALL(C_SeedRandom)
		    CASE_CALL(C_GenerateRandom)
#undef CASE_CALL
	default:
		/* This should have been caught by the parse code */
		assert(0 && "Unchecked call");
		break;
	};

	if (ret == CKR_OK) {

		/* Parsing errors? */
		if (gck_rpc_message_buffer_error(req)) {
			gck_rpc_warn
			    ("invalid request from module, probably too short");
			ret = PARSE_ERROR;
		}

		/* Out of memory errors? */
		if (gck_rpc_message_buffer_error(resp)) {
			gck_rpc_warn
			    ("out of memory error putting together message");
			ret = PREP_ERROR;
		}
	}

	/* A filled in response */
	if (ret == CKR_OK) {

		/*
		 * Since we're dealing with many many functions above generating
		 * these messages we want to make sure each of them actually
		 * does what it's supposed to.
		 */

		assert(gck_rpc_message_is_verified(resp));
		assert(resp->call_type == GCK_RPC_RESPONSE);
		assert(resp->call_id == req->call_id);
		assert(gck_rpc_calls[resp->call_id].response);
		assert(strcmp(gck_rpc_calls[resp->call_id].response,
			      resp->signature) == 0);

		/* Fill in an error respnose */
	} else {
		if (!gck_rpc_message_prep
		    (resp, GCK_RPC_CALL_ERROR, GCK_RPC_RESPONSE)
		    || !gck_rpc_message_write_ulong(resp, (uint32_t) ret)
		    || gck_rpc_message_buffer_error(resp)) {
			gck_rpc_warn("out of memory responding with error");
			return 0;
		}
	}

	return 1;
}

static int read_all(CallState *cs, void *data, size_t len)
{
	int r;

	assert(cs->sock >= 0);
	assert(data);
	assert(len > 0);

	while (len > 0) {

		if (cs->tls)
			r = gck_rpc_tls_read_all(cs->tls, data, len);
		else
			r = recv(cs->sock, data, len, 0);

		if (r == 0) {
			/* Connection was closed on client */
			return 0;
		} else if (r == -1) {
			if (errno != EAGAIN && errno != EINTR) {
				gck_rpc_warn("couldn't receive data: %s",
					     strerror(errno));
				return 0;
			}
		} else {
			data += r;
			len -= r;
		}
	}
	return 1;
}

static int write_all(CallState *cs, void *data, size_t len)
{
	int r;

	assert(cs->sock >= 0);
	assert(data);
	assert(len > 0);

	while (len > 0) {

		if (cs->tls)
			r = gck_rpc_tls_write_all(cs->tls, (void *) data, len);
		else
                        r = send(cs->sock, data, len, MSG_NOSIGNAL);

		if (r == -1) {
			if (errno == EPIPE) {
				/* Connection closed from client */
				return 0;
			} else if (errno != EAGAIN && errno != EINTR) {
				gck_rpc_warn("couldn't send data: %s",
					     strerror(errno));
				return 0;
			}
		} else {
			data += r;
			len -= r;
		}
	}

	return 1;
}

static void run_dispatch_loop(CallState *cs)
{
	unsigned char buf[4];
	uint32_t len, res;
	char hoststr[NI_MAXHOST], portstr[NI_MAXSERV];

	assert(cs->sock != -1);

	if ((res = getnameinfo((struct sockaddr *) & cs->addr, cs->addrlen,
			       hoststr, sizeof(hoststr), portstr, sizeof(portstr),
			       NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
		gck_rpc_warn("couldn't call getnameinfo on client addr: %.100s",
			     gai_strerror(res));
		hoststr[0] = portstr[0] = '\0';
	}

	/* Enable TLS for this socket */
	if (cs->tls) {
		if (! gck_rpc_start_tls(cs->tls, cs->sock)) {
			gck_rpc_warn("Can't enable TLS");
			return ;
		}
	}

	/* The client application */
	if (! cs->read(cs, (void *)&cs->appid, sizeof (cs->appid))) {
		gck_rpc_warn("Can't read appid\n");
		return ;
	}

	gck_rpc_log("New session %d-%d (client %s, port %s)\n", (uint32_t) (cs->appid >> 32),
		    (uint32_t) cs->appid, hoststr, portstr);

	/* Setup our buffers */
	if (!call_init(cs)) {
		gck_rpc_warn("out of memory");
		return;
	}

	/* The main thread loop */
	while (TRUE) {

		call_reset(cs);

		/* Read the number of bytes ... */
		if (! cs->read(cs, buf, 4))
			break;

		/* Calculate the number of bytes */
		len = egg_buffer_decode_uint32(buf);
		if (len >= 0x0FFFFFFF) {
			gck_rpc_warn
			    ("invalid message size from module: %u bytes", len);
			break;
		}

		/* Allocate memory */
		egg_buffer_reserve(&cs->req->buffer, cs->req->buffer.len + len);
		if (egg_buffer_has_error(&cs->req->buffer)) {
			gck_rpc_warn("error allocating buffer for message");
			break;
		}

		/* ... and read/parse in the actual message */
		if (!cs->read(cs, cs->req->buffer.buf, len))
			break;

		egg_buffer_add_empty(&cs->req->buffer, len);

		if (!gck_rpc_message_parse(cs->req, GCK_RPC_REQUEST))
			break;

		/* ... send for processing ... */
		if (!dispatch_call(cs))
			break;

		/* .. send back response length, and then response data */
		egg_buffer_encode_uint32(buf, cs->resp->buffer.len);
		if (!cs->write(cs, buf, 4) ||
		    !cs->write(cs, cs->resp->buffer.buf, cs->resp->buffer.len))
			break;
	}

	call_uninit(cs);
}

static void *run_dispatch_thread(void *arg)
{
	CallState *cs = arg;
	assert(cs->sock != -1);

	if (_install_dispatch_syscall_filter((cs->tls != NULL)))
		return NULL;

	run_dispatch_loop(cs);

	/* The thread closes the socket and marks as done */
	assert(cs->sock != -1);
	close(cs->sock);
	cs->sock = -1;

	return NULL;
}

/* ---------------------------------------------------------------------------
 * MAIN THREAD
 */

/* The main daemon socket that we're listening on */
static int pkcs11_socket = -1;

/* The unix socket path, that we listen on */
static char pkcs11_socket_path[MAXPATHLEN] = { 0, };

void gck_rpc_layer_accept(GckRpcTlsPskState *tls)
{
	struct sockaddr_storage addr;
	DispatchState *ds, **here;
	int error;
	socklen_t addrlen;
	int new_fd;

	assert(pkcs11_socket != -1);

	/* Cleanup any completed dispatch threads */
	pthread_mutex_lock(&pkcs11_dispatchers_mutex);
	for (here = &pkcs11_dispatchers, ds = *here; ds != NULL; ds = *here) {
		CallState *c = &ds->cs;
		if (c && c->sock == -1) {
			pthread_join(ds->thread, NULL);
			*here = ds->next;
			free(ds);
		} else {
			here = &ds->next;
		}
	}

	addrlen = sizeof(addr);
	new_fd = accept(pkcs11_socket, (struct sockaddr *)&addr, &addrlen);
	if (new_fd < 0) {
		gck_rpc_warn("cannot accept pkcs11 connection: %s",
			     strerror(errno));
		return;
	}

	ds = calloc(1, sizeof(DispatchState));
	if (ds == NULL) {
		gck_rpc_warn("out of memory");
		close(new_fd);
		return;
	}

	ds->cs.sock = new_fd;
        ds->cs.read = &read_all;
        ds->cs.write = &write_all;
	ds->cs.addr = addr;
	ds->cs.addrlen = addrlen;
	ds->cs.tls = tls;

	error = pthread_create(&ds->thread, NULL,
			       run_dispatch_thread, &(ds->cs));
	if (error) {
		gck_rpc_warn("couldn't start thread: %s", strerror(errno));
		close(new_fd);
		free(ds);
		return;
	}

	ds->next = pkcs11_dispatchers;
	pkcs11_dispatchers = ds;
	pthread_mutex_unlock(&pkcs11_dispatchers_mutex);
}

static int _inetd_read(CallState *cs, void *data, size_t len)
{
	assert(cs->sock >= 0);
	return read(cs->sock, data, len);
}

static int _inetd_write(CallState *cs, void *data, size_t len)
{
	assert(cs->sock >= 0);
	return write(cs->sock, data, len);
}

void gck_rpc_layer_inetd(CK_FUNCTION_LIST_PTR module)
{
   CallState cs;

   memset(&cs, 0, sizeof(cs));
   cs.sock = STDIN_FILENO;
   cs.read = &_inetd_read;
   cs.write = &_inetd_write;

   pkcs11_module = module;

   run_dispatch_thread(&cs);
}

/*
 * Try to get a listening socket for host and port (host may be either a name or
 * an IP address, as string) and port (a string with a service name or a port
 * number).
 *
 * Returns -1 on failure, and the socket fd otherwise.
 */
static int _get_listening_socket(const char *proto, const char *host, const char *port)
{
	char hoststr[NI_MAXHOST], portstr[NI_MAXSERV];
	struct addrinfo *ai, *first, hints;
	int res, sock, one = 1;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;		/* Want addr for bind() */
	hints.ai_family = AF_UNSPEC;		/* Either IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;	/* Only stream oriented sockets */

	if ((res = getaddrinfo(host, port, &hints, &ai)) < 0) {
		gck_rpc_warn("couldn't resolve host '%.100s' or service '%.100s' : %.100s\n",
			     host, port, gai_strerror(res));
		return -1;
	}

	sock = -1;
	first = ai;

	/* Loop through the sockets returned and see if we can find one that accepts
	 * our options and bind()
	 */
	while (ai) {
		sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

		if (sock >= 0) {
			if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
				       (char *)&one, sizeof (one)) == -1) {
				gck_rpc_warn("couldn't set pkcs11 "
					     "socket protocol options (%.100s %.100s): %.100s",
					     host, port, strerror (errno));
				goto next;
			}

			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
				       (char *)&one, sizeof(one)) == -1) {
				gck_rpc_warn
					("couldn't set pkcs11 socket options (%.100s %.100s): %.100s",
					 host, port, strerror(errno));
				goto next;
			}

			if (bind(sock, ai->ai_addr, ai->ai_addrlen) == 0)
				break;

		next:
			close(sock);
			sock = -1;
		}
		ai = ai->ai_next;
	}

	if (sock < 0) {
		gck_rpc_warn("couldn't create pkcs11 socket (%.100s %.100s): %.100s\n",
			     host, port, strerror(errno));
		sock = -1;
		goto out;
	}

	if (listen(sock, PKCS11PROXY_LISTEN_BACKLOG) < 0) {
		gck_rpc_warn("couldn't listen on pkcs11 socket (%.100s %.100s): %.100s",
			     host, port, strerror(errno));
		sock = -1;
		goto out;
	}

	/* Format a string describing the socket we're listening on into pkcs11_socket_path */
	if ((res = getnameinfo(ai->ai_addr, ai->ai_addrlen,
			       hoststr, sizeof(hoststr), portstr, sizeof(portstr),
			       NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
		gck_rpc_warn("couldn't call getnameinfo on pkcs11 socket (%.100s %.100s): %.100s",
			     host, port, gai_strerror(res));
		sock = -1;
		goto out;
	}

	snprintf(pkcs11_socket_path, sizeof(pkcs11_socket_path),
		 (ai->ai_family == AF_INET6) ? "%s://[%s]:%s" : "%s://%s:%s", proto, hoststr, portstr);

 out:
	freeaddrinfo(first);

	return sock;
}

int gck_rpc_layer_initialize(const char *prefix, CK_FUNCTION_LIST_PTR module)
{
	struct sockaddr_un addr;
	int sock;

#ifdef _DEBUG
	GCK_RPC_CHECK_CALLS();
#endif

	assert(module);
	assert(prefix);

	/* cannot be called more than once */
	assert(!pkcs11_module);
	assert(pkcs11_socket == -1);
	assert(pkcs11_dispatchers == NULL);

	memset(&addr, 0, sizeof(addr));

#ifdef  __MINGW32__
        {
		WSADATA wsaData;
		int iResult;

		iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != 0) {
			gck_rpc_warn("WSAStartup failed: %d\n", iResult);
			return -1;
		}
        }
#endif

	if (!strncmp("tcp://", prefix, 6) ||
	    !strncmp("tls://", prefix, 6)) {
		/*
		 * TCP socket
		 */
		char *host, *port;
		char proto[4]; /* strlen("tcp") and strlen("tls") */

		snprintf(proto, sizeof(proto), "%s", prefix);

		if (! gck_rpc_parse_host_port(prefix + 6, &host, &port)) {
			free(host);
			return -1;
		}

		if ((sock = _get_listening_socket(proto, host, port)) == -1) {
			free(host);
			return -1;
		}

		free(host);
	} else {
		/*
		 * UNIX domain socket
		 */
		snprintf(pkcs11_socket_path, sizeof(pkcs11_socket_path),
			 "%s/socket.pkcs11", prefix);

		sock = socket(AF_UNIX, SOCK_STREAM, 0);

		if (sock < 0) {
			gck_rpc_warn("couldn't create pkcs11 socket: %s",
				     strerror(errno));
			return -1;
		}

		addr.sun_family = AF_UNIX;
		unlink(pkcs11_socket_path);
		strncpy(addr.sun_path, pkcs11_socket_path,
			sizeof(addr.sun_path));

		if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			gck_rpc_warn("couldn't bind to pkcs11 socket: %s: %s",
				     pkcs11_socket_path, strerror(errno));
			return -1;
		}

		if (listen(sock, PKCS11PROXY_LISTEN_BACKLOG) < 0) {
			gck_rpc_warn("couldn't listen on pkcs11 socket: %s: %s",
				     pkcs11_socket_path, strerror(errno));
			return -1;
		}
	}

	gck_rpc_log("Listening on: %s\n", pkcs11_socket_path);

	pkcs11_module = module;
	pkcs11_socket = sock;
	pkcs11_dispatchers = NULL;

	return sock;
}

void gck_rpc_layer_uninitialize(void)
{
	DispatchState *ds, *next;

	if (!pkcs11_module)
		return;

	/* Close our main listening socket */
	if (pkcs11_socket != -1)
		close(pkcs11_socket);
	pkcs11_socket = -1;

	/* Delete our unix socket */
	if (pkcs11_socket_path[0] &&
	    strncmp(pkcs11_socket_path, "tcp://", strlen("tcp://")) != 0 &&
	    strncmp(pkcs11_socket_path, "tls://", strlen("tls://")) != 0)
		unlink(pkcs11_socket_path);
	pkcs11_socket_path[0] = 0;

	/* Stop all of the dispatch threads */
	pthread_mutex_lock(&pkcs11_dispatchers_mutex);
	for (ds = pkcs11_dispatchers; ds; ds = next) {
		CallState *c = &ds->cs;
		next = ds->next;

		/* Forcibly shutdown the connection */
		if (c && c->sock != -1)
			if (shutdown(c->sock, SHUT_RDWR) == 0)
				c->sock = -1;

		pthread_join(ds->thread, NULL);

		/* This is always closed by dispatch thread */
		if (c)
			assert(c->sock == -1);
		free(ds);
	}
	pthread_mutex_unlock(&pkcs11_dispatchers_mutex);

	pkcs11_module = NULL;
}

/*
 * Reduce the syscalls allowed to a subset of the syscalls allowed for
 * the parent thread.
 */
static int _install_dispatch_syscall_filter(int use_tls)
{
#ifdef SECCOMP
	int rc = -1;
	scmp_filter_ctx ctx;

#ifdef DEBUG_SECCOMP
	ctx = seccomp_init(SCMP_ACT_TRAP);
#else
	ctx = seccomp_init(SCMP_ACT_KILL);
#endif /* DEBUG_SECCOMP */
	if (ctx == NULL)
		goto failure_scmp;
	/*
	 * These are the basic syscalls needed to be able to use
	 * the syscall-reporter to figure out the rest
	 */
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
#ifdef DEBUG_SECCOMP
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
# ifdef __NR_sigreturn
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(sigreturn), 0);
# endif
#endif /* DEBUG_SECCOMP */
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

	/*
	 * Network related syscalls.
	 */
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(sendto), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(recvfrom), 0);

	/*
	 * TLS-PSK
	 */
	if (use_tls)
		/* Allow open() of the TLS-PSK keyfile. */
		seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
				 SCMP_A1(SCMP_CMP_EQ, O_RDONLY | O_CLOEXEC));

	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(close), 0);

	/*
	 * pthreads?
	 */
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(madvise), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 1,
			 SCMP_A2(SCMP_CMP_EQ, PROT_READ|PROT_WRITE));

	/*
	 * SoftHSM 1.3.0
	 */
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(getcwd), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(stat), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(access), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(fsync), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(unlink), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(ftruncate), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(select), 0);
	seccomp_rule_add(ctx,SCMP_ACT_ALLOW, SCMP_SYS(futex), 0);

	rc = seccomp_load(ctx);
	if (rc < 0)
		goto failure_scmp;
	seccomp_release(ctx);

	return 0;

failure_scmp:
	errno = -rc;
	gck_rpc_warn("Seccomp filter initialization failed, errno = %u\n", errno);
	return errno;
#else /* SECCOMP */
        return 0;
#endif /* SECCOMP */
}
