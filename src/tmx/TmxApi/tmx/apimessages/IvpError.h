/*
 * IvpError.h
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#ifndef IVPERROR_H_
#define IVPERROR_H_

#include "../tmx.h"
#include "../IvpMessage.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * Error Message Levels
 */
typedef enum {
	IvpLogLevel_debug = 0,
	IvpLogLevel_info,
	IvpLogLevel_warn,
	IvpLogLevel_error,
	IvpLogLevel_fatal
} IvpLogLevel;

typedef enum {
	IvpError_none = 0,
	IvpError_manifestLoad,
	IvpError_manifestParse,
	IvpError_manifestExtract,
	IvpError_pluginNoName,
	IvpError_configFormat,
	IvpError_configDuplicateKey,
	IvpError_configMissingKey,
	IvpError_configMissingDefault,
	IvpError_connectFail,
	IvpError_connectionDropped,
	IvpError_configKeyDoesntExist,
	IvpError_messageParse,
	IvpError_assert,
	IvpError_msgTypeCollectionFormat,
	IvpError_msgTypeCollectionMissingType,
	IvpError_msgTypeCollectionMissingSubType,
	IvpError_msgTypeCollectionDuplicate
} IvpErrorNumber;

/*!
 * Error definition
 */
typedef struct {
	IvpLogLevel level;
	IvpErrorNumber error;
	int sysErrNo;
} IvpError;

/*!
 * A static initializer to create a safe IvpError.
 */
extern const IvpError IVP_ERROR_INITIALIZER;

/*!
 * Creates a new error from arguments.
 *
 * @param level
 * 		The level to include in the error.
 *
 * @param error
 * 		The IvpError number.
 *
 * @param sysErrNo
 * 		None zero system error number to print at the end of the error description.
 *
 * @returns
 * 		The error structure.
 */
IvpError ivpError_createError(IvpLogLevel level, IvpErrorNumber error, int sysErrNo);

/*!
 * Determines if the given message is an error message.
 *
 * @param msg
 * 		The message to determine if it is an error message.
 *
 * @returns
 * 		non-zero if it's an error message, or zero if it's not.
 *
 * @requires
 * 		msg != NULL
 */
int ivpError_isErrMsg(IvpMessage *msg);


IvpMessage *ivpError_createMsg(IvpError err);

/*!
 * Extracts the error from an IvpMessage.
 *
 * @param msg
 * 		The message to extract the error details out of.
 *
 * @returns
 * 		The error message details contained in the IvpMessage.
 *
 * @requires
 * 		msg != NULL
 * 		ivpError_isErrMsg(msg) != 0
 */
IvpError ivpError_getError(IvpMessage *msg);


#ifdef __cplusplus
}
#endif


#endif /* IVPERROR_H_ */
