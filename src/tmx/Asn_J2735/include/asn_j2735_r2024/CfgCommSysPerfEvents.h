/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeDataConfig"
 * 	found in "J2945-C-ProbeDataConfig-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_CfgCommSysPerfEvents_H_
#define	_CfgCommSysPerfEvents_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeInteger.h"
#include "BOOLEAN.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CfgCommSysPerfEvents */
typedef struct CfgCommSysPerfEvents {
	long	*j2945_1ChanBusyThresh;	/* OPTIONAL */
	BOOLEAN_t	*rfDataRsuInfo;	/* OPTIONAL */
	long	*numRsusObservedThresh;	/* OPTIONAL */
	long	*rfV2xJamDetectThresh;	/* OPTIONAL */
	long	*j2945_1VehDensThresh;	/* OPTIONAL */
	long	*j2945_1CqiBelowThresh;	/* OPTIONAL */
	long	*j2945_1TrackErrorThresh;	/* OPTIONAL */
	long	*gnssHdopExceedsThresh;	/* OPTIONAL */
	long	*gnssSatsBelowThresh;	/* OPTIONAL */
	BOOLEAN_t	*gnssJammingDetect;	/* OPTIONAL */
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CfgCommSysPerfEvents_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CfgCommSysPerfEvents;
extern asn_SEQUENCE_specifics_t asn_SPC_CfgCommSysPerfEvents_specs_1;
extern asn_TYPE_member_t asn_MBR_CfgCommSysPerfEvents_1[10];

#ifdef __cplusplus
}
#endif

#endif	/* _CfgCommSysPerfEvents_H_ */
#include "asn_internal.h"