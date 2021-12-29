/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* p11-rpc-message.c - our marshalled PKCS#11 protocol.

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

#include <string.h>

#ifdef G_DISABLE_ASSERT
#define assert(x)
#else
#include <assert.h>
#endif

GckRpcMessage *gck_rpc_message_new(EggBufferAllocator allocator)
{
	GckRpcMessage *msg;

	assert(allocator);

	msg = (GckRpcMessage *) (allocator) (NULL, sizeof(GckRpcMessage));
	if (!msg)
		return NULL;
	memset(msg, 0, sizeof(*msg));

	if (!egg_buffer_init_full(&msg->buffer, 64, allocator)) {
		(allocator) (msg, 0);	/* Frees allocation */
		return NULL;
	}

	gck_rpc_message_reset(msg);

	return msg;
}

void gck_rpc_message_free(GckRpcMessage * msg)
{
	EggBufferAllocator allocator;

	if (msg) {
		assert(msg->buffer.allocator);
		allocator = msg->buffer.allocator;
		egg_buffer_uninit(&msg->buffer);

		/* frees data buffer */
		(allocator) (msg, 0);
	}
}

void gck_rpc_message_reset(GckRpcMessage * msg)
{
	assert(msg);

	msg->call_id = 0;
	msg->call_type = 0;
	msg->signature = NULL;
	msg->sigverify = NULL;
	msg->parsed = 0;

	egg_buffer_reset(&msg->buffer);
}

int
gck_rpc_message_prep(GckRpcMessage * msg, int call_id, GckRpcMessageType type)
{
	int len;

	assert(type);
	assert(call_id >= GCK_RPC_CALL_ERROR);
	assert(call_id < GCK_RPC_CALL_MAX);

	gck_rpc_message_reset(msg);

	if (call_id != GCK_RPC_CALL_ERROR) {

		/* The call id and signature */
		if (type == GCK_RPC_REQUEST)
			msg->signature = gck_rpc_calls[call_id].request;
		else if (type == GCK_RPC_RESPONSE)
			msg->signature = gck_rpc_calls[call_id].response;
		else
			assert(0 && "invalid message type");
		assert(msg->signature);
		msg->sigverify = msg->signature;
	}

	msg->call_id = call_id;
	msg->call_type = type;

	/* Encode the two of them */
	egg_buffer_add_uint32(&msg->buffer, call_id);
	if (msg->signature) {
		len = strlen(msg->signature);
		egg_buffer_add_byte_array(&msg->buffer,
					  (unsigned char *)msg->signature, len);
	}

	msg->parsed = 0;
	return !egg_buffer_has_error(&msg->buffer);
}

int gck_rpc_message_parse(GckRpcMessage * msg, GckRpcMessageType type)
{
	const unsigned char *val;
	size_t len;
	uint32_t call_id;

	msg->parsed = 0;

	/* Pull out the call identifier */
	if (!egg_buffer_get_uint32
	    (&msg->buffer, msg->parsed, &(msg->parsed), &call_id)) {
		gck_rpc_warn("invalid message: couldn't read call identifier");
		return 0;
	}

	msg->signature = msg->sigverify = NULL;

	/* If it's an error code then no more processing */
	if (call_id == GCK_RPC_CALL_ERROR) {
		if (type == GCK_RPC_REQUEST) {
			gck_rpc_warn("invalid message: error code in request");
			return 0;
		}

		return 1;
	}

	/* The call id and signature */
	if (call_id <= 0 || call_id >= GCK_RPC_CALL_MAX) {
		gck_rpc_warn("invalid message: bad call id: %d", call_id);
		return 0;
	}
	if (type == GCK_RPC_REQUEST)
		msg->signature = gck_rpc_calls[call_id].request;
	else if (type == GCK_RPC_RESPONSE)
		msg->signature = gck_rpc_calls[call_id].response;
	else
		assert(0 && "invalid message type");
	msg->call_id = call_id;
	msg->call_type = type;
	msg->sigverify = msg->signature;

	/* Verify the incoming signature */
	if (!egg_buffer_get_byte_array
	    (&msg->buffer, msg->parsed, &(msg->parsed), &val, &len)) {
		gck_rpc_warn("invalid message: couldn't read signature");
		return 0;
	}

	if ((strlen(msg->signature) != len)
	    || (memcmp(val, msg->signature, len) != 0)) {
		gck_rpc_warn("invalid message: signature doesn't match");
		return 0;
	}

	return 1;
}

int gck_rpc_message_equals(GckRpcMessage * m1, GckRpcMessage * m2)
{
	assert(m1 && m2);

	/* Any errors and messages are never equal */
	if (egg_buffer_has_error(&m1->buffer) ||
	    egg_buffer_has_error(&m2->buffer))
		return 0;

	/* Calls and signatures must be identical */
	if (m1->call_id != m2->call_id)
		return 0;
	if (m1->call_type != m2->call_type)
		return 0;
	if (m1->signature && m2->signature) {
		if (strcmp(m1->signature, m2->signature) != 0)
			return 0;
	} else if (m1->signature != m2->signature) {
		return 0;
	}

	/* Data in buffer must be identical */
	return egg_buffer_equal(&m1->buffer, &m2->buffer);
}

int gck_rpc_message_verify_part(GckRpcMessage * msg, const char *part)
{
	int len, ok;

	if (!msg->sigverify)
		return 1;

	len = strlen(part);
	ok = (strncmp(msg->sigverify, part, len) == 0);
	if (ok)
		msg->sigverify += len;
	return ok;
}

int
gck_rpc_message_write_attribute_buffer(GckRpcMessage * msg,
				       CK_ATTRIBUTE_PTR arr, CK_ULONG num)
{
	CK_ATTRIBUTE_PTR attr;
	CK_ULONG i;

	assert(!num || arr);
	assert(msg);

	/* Make sure this is in the rigth order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "fA"));

	/* Write the number of items */
	egg_buffer_add_uint32(&msg->buffer, num);

	for (i = 0; i < num; ++i) {
		attr = &(arr[i]);

		/* The attribute type */
		egg_buffer_add_uint32(&msg->buffer, attr->type);

		/* And the attribute buffer length */
		egg_buffer_add_uint32(&msg->buffer,
				      attr->pValue ? attr->ulValueLen : 0);
	}

	return !egg_buffer_has_error(&msg->buffer);
}

int
gck_rpc_message_write_attribute_array(GckRpcMessage * msg,
				      CK_ATTRIBUTE_PTR arr, CK_ULONG num)
{
	CK_ULONG i;
	CK_ATTRIBUTE_PTR attr;
	unsigned char validity;

	assert(!num || arr);
	assert(msg);

	/* Make sure this is in the rigth order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "aA"));

	/* Write the number of items */
	egg_buffer_add_uint32(&msg->buffer, num);

	for (i = 0; i < num; ++i) {
		attr = &(arr[i]);

		/* The attribute type */
		egg_buffer_add_uint32(&msg->buffer, attr->type);

		/* Write out the attribute validity */
		validity = (((CK_LONG) attr->ulValueLen) == -1) ? 0 : 1;
		egg_buffer_add_byte(&msg->buffer, validity);

		/* The attribute length and value */
		if (validity) {
			egg_buffer_add_uint32(&msg->buffer, attr->ulValueLen);
			if (gck_rpc_has_bad_sized_ulong_parameter(attr)) {
				uint64_t val = *(CK_ULONG *)attr->pValue;

				egg_buffer_add_byte_array (&msg->buffer, (unsigned char *)&val, sizeof (val));
			} else
				egg_buffer_add_byte_array(&msg->buffer, attr->pValue,
							  attr->ulValueLen);
		}
	}

	return !egg_buffer_has_error(&msg->buffer);
}

int gck_rpc_message_read_byte(GckRpcMessage * msg, CK_BYTE * val)
{
	assert(msg);

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "y"));
	return egg_buffer_get_byte(&msg->buffer, msg->parsed, &msg->parsed,
				   val);
}

int gck_rpc_message_write_byte(GckRpcMessage * msg, CK_BYTE val)
{
	assert(msg);

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "y"));
	return egg_buffer_add_byte(&msg->buffer, val);
}

int gck_rpc_message_read_ulong(GckRpcMessage * msg, CK_ULONG * val)
{
	uint64_t v;
	assert(msg);

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "u"));

	if (!egg_buffer_get_uint64(&msg->buffer, msg->parsed, &msg->parsed, &v))
		return 0;
	if (val)
		*val = (CK_ULONG) v;
	return 1;
}

int gck_rpc_message_write_ulong(GckRpcMessage * msg, CK_ULONG val)
{
	assert(msg);

	/* Make sure this is in the rigth order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "u"));
	return egg_buffer_add_uint64(&msg->buffer, val);
}

int gck_rpc_message_write_byte_buffer(GckRpcMessage * msg, CK_BYTE_PTR arr, CK_ULONG *count_ptr)
{
	uint8_t flags;
	assert(msg);

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "fy"));

	flags = 0;
	if (! arr)
		flags |= GCK_RPC_BYTE_BUFFER_NULL_DATA;
	if (! count_ptr)
		flags |= GCK_RPC_BYTE_BUFFER_NULL_COUNT;

	egg_buffer_add_byte(&msg->buffer, flags);

	egg_buffer_add_uint32(&msg->buffer, count_ptr ? *count_ptr : 0x0);

	return !egg_buffer_has_error(&msg->buffer);
}

int
gck_rpc_message_write_byte_array(GckRpcMessage * msg, CK_BYTE_PTR arr,
				 CK_ULONG num)
{
	assert(msg);

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "ay"));

	/* No array, no data, just length */
	if (!arr) {
		egg_buffer_add_byte(&msg->buffer, 0);
		egg_buffer_add_uint32(&msg->buffer, num);
	} else {
		egg_buffer_add_byte(&msg->buffer, 1);
		egg_buffer_add_byte_array(&msg->buffer, arr, num);
	}

	return !egg_buffer_has_error(&msg->buffer);
}

int gck_rpc_message_write_ulong_buffer(GckRpcMessage * msg, CK_ULONG count)
{
	assert(msg);

	/* Make sure this is in the right order */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "fu"));
	return egg_buffer_add_uint32(&msg->buffer, count);
}

int
gck_rpc_message_write_ulong_array(GckRpcMessage * msg, CK_ULONG_PTR array,
				  CK_ULONG n_array)
{
	CK_ULONG i;

	assert(msg);

	/* Check that we're supposed to have this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "au"));

	/* We send a byte which determines whether there's actual data present or not */
	egg_buffer_add_byte(&msg->buffer, array ? 1 : 0);
	egg_buffer_add_uint32(&msg->buffer, n_array);

	/* Now send the data if valid */
	if (array) {
		for (i = 0; i < n_array; ++i)
			egg_buffer_add_uint64(&msg->buffer, array[i]);
	}

	return !egg_buffer_has_error(&msg->buffer);
}

int gck_rpc_message_read_version(GckRpcMessage * msg, CK_VERSION * version)
{
	assert(msg);
	assert(version);

	/* Check that we're supposed to have this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "v"));

	return egg_buffer_get_byte(&msg->buffer, msg->parsed, &msg->parsed,
				   &version->major)
	    && egg_buffer_get_byte(&msg->buffer, msg->parsed, &msg->parsed,
				   &version->minor);
}

int gck_rpc_message_write_version(GckRpcMessage * msg, CK_VERSION * version)
{
	assert(msg);
	assert(version);

	/* Check that we're supposed to have this at this point */
	assert(!msg->signature || gck_rpc_message_verify_part(msg, "v"));

	egg_buffer_add_byte(&msg->buffer, version->major);
	egg_buffer_add_byte(&msg->buffer, version->minor);

	return !egg_buffer_has_error(&msg->buffer);
}

int
gck_rpc_message_read_space_string(GckRpcMessage * msg, CK_UTF8CHAR * buffer,
				  CK_ULONG length)
{
	const unsigned char *data;
	size_t n_data;

	assert(msg);
	assert(buffer);
	assert(length);

	assert(!msg->signature || gck_rpc_message_verify_part(msg, "s"));

	if (!egg_buffer_get_byte_array
	    (&msg->buffer, msg->parsed, &msg->parsed, &data, &n_data))
		return 0;

	if (n_data != length) {
		gck_rpc_warn
		    ("invalid length space padded string received: %d != %d",
		     length, n_data);
		return 0;
	}

	memcpy(buffer, data, length);
	return 1;
}

int
gck_rpc_message_write_space_string(GckRpcMessage * msg, CK_UTF8CHAR * buffer,
				   CK_ULONG length)
{
	assert(msg);
	assert(buffer);
	assert(length);

	assert(!msg->signature || gck_rpc_message_verify_part(msg, "s"));

	/* XXX it's not really right to treat UTF-8 input as a byte buffer,
	 * although CK_UTF8CHAR is currently typedef'd to unsigned char in pkcs11.h.
	 */
	return egg_buffer_add_byte_array(&msg->buffer, buffer, length);
}
