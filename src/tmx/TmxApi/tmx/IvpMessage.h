/*
 * IvpMessage.h
 *
 *  Created on: Jul 17, 2014
 *      Author: ivp
 */

#ifndef ivpMsg_H_
#define ivpMsg_H_

#include "json/cJSONxtra.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned int IvpMsgFlags;
#define IvpMsgFlags_None 0x00
#define IvpMsgFlags_RouteDSRC 0x01

typedef struct IvpDsrcMetadata {
	int channel;
	int psid;
} IvpDsrcMetadata;

typedef struct IvpMessage {
	char *type;
	char *subtype;
	char *source;
	unsigned int sourceId;
	char *encoding;
	uint64_t timestamp;
	IvpMsgFlags flags;
	IvpDsrcMetadata *dsrcMetadata;
	cJSON *payload;
} IvpMessage;

typedef enum {
	IvpMsg_FormatOptions_none = 0x00,
	IvpMsg_FormatOptions_formatted = 0x01
} IvpMsg_FormatOptions;

/*!
 * Creates a new IvpMessage from arguments.
 *
 * @param type
 * 		Null terminated message type string.  String is copied.  NULL is allowed.
 *
 * @param subtype
 * 		Null terminated message sub-type string.  String is copied.  NULL is allowed.
 *
 * @param encoding
 * 		Null terminated message encoding string.  String is copied.  NULL is allowed.
 *
 * @param flags
 * 		Bit-or'ed flags.
 *
 * @param
 * 		json payload.  cJSON Object is copied.  NULL is allowed.
 *
 * @returns
 * 		A malloc'ed IvpMessage or NULL if an error occurred.
 */
IvpMessage *ivpMsg_create(const char *type, const char *subtype, const char *encoding, IvpMsgFlags flags, cJSON *payload);

IvpMessage *ivpMsg_addDsrcMetadata(IvpMessage *msg, int channel, int psid);

/*!
 * Creates a new IvpMessage from a json string.
 *
 * @param jsonmsg
 * 		Null terminated json string.
 *
 * @returns
 * 		A malloc'ed IvpMessage or NULL if an error occurred.
 *
 * @requires
 * 		jsonmsg != NULL
 */
IvpMessage *ivpMsg_parse(char *jsonmsg);

/*!
 * Creates a copy of an IvpMessage.
 *
 * @param msg
 * 		The message to make a copy of.
 *
 * @returns
 * 		A new malloc'ed IvpMessage or NULL if an error occurred.
 *
 * @requires
 * 		msg != NULL
 */
IvpMessage *ivpMsg_copy(IvpMessage *msg);

/*!
 * Creates a json message representation of an IvpMessage.
 *
 * @param msg
 * 		The message to create the json message of.
 *
 * @param options
 * 		Format options about how to create the json message.
 *
 * @returns
 * 		A new malloc'ed null terminated string or NULL if an error occurred.
 *
 * @requires
 * 		msg != NULL
 */
char *ivpMsg_createJsonString(IvpMessage *msg, IvpMsg_FormatOptions options);

/*!
 * Properly destroys and free's an IvpMessage.  All IvpMessages should be destroyed using this function.
 *
 * @param msg
 * 		The message to destroy.
 *
 * @requires
 * 		msg != NULL
 */
void ivpMsg_destroy(IvpMessage *msg);

#ifdef __cplusplus
}
#endif

#endif /* ivpMsg_H_ */
