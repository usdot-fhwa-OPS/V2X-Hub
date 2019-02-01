/*
 * ivp.h
 *
 *  Created on: Jul 21, 2014
 *      Author: ivp
 */

#ifndef IVP_H_
#define IVP_H_

#define IVPMSG_TYPE_APIRESV_SUBSCRIBE "__subscribe"
#define IVPMSG_TYPE_APIRESV_REGISTER "__register"
#define IVPMSG_TYPE_APIRESV_ERROR "__error"
#define IVPMSG_TYPE_APIRESV_STATUS "__status"
#define IVPMSG_TYPE_APIRESV_CONFIG "__config"
#define IVPMSG_TYPE_APIRESV_EVENTLOG "__eventLog"

#define IVP_STATUS_UNKNOWN "Unknown"
#define IVP_STATUS_STARTED "Started, waiting for connection..."
#define IVP_STATUS_RUNNING "Running"
#define IVP_STATUS_STALE "Connection going stale"
#define IVP_STATUS_STOPPED_DISCONENCTED "Stopped / Disconnected"

#define IVP_DEFAULT_IP "127.0.0.1"
#define IVP_DEFAULT_PORT 24601

#define IVP_ENCODING_ASN1_BER "asn.1-ber/hexstring"
#define IVP_ENCODING_ASN1_UPER "asn.1-uper/hexstring"
#define IVP_ENCODING_JSON "json"
#define IVP_ENCODING_STRING "string"
#define IVP_ENCODING_BYTEARRAY "bytearray/hexstring"


#endif /* IVP_H_ */
