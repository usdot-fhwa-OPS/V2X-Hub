/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "BasicSafetyMessage"
 * 	found in "J2735-BasicSafetyMessage-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_BasicSafetyMessage_H_
#define	_BasicSafetyMessage_H_


#include "asn_application.h"

/* Including external dependencies */
#include "BSMcoreData.h"
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct BSMpartIIExtension;
struct Reg_BasicSafetyMessage;

/* BasicSafetyMessage */
typedef struct BasicSafetyMessage {
	BSMcoreData_t	 coreData;
	struct BasicSafetyMessage__partII {
		A_SEQUENCE_OF(struct BSMpartIIExtension) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *partII;
	struct BasicSafetyMessage__regional {
		A_SEQUENCE_OF(struct Reg_BasicSafetyMessage) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *regional;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BasicSafetyMessage_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BasicSafetyMessage;
extern asn_SEQUENCE_specifics_t asn_SPC_BasicSafetyMessage_specs_1;
extern asn_TYPE_member_t asn_MBR_BasicSafetyMessage_1[3];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "PartIIcontent.h"
#include "RegionalExtension.h"

#endif	/* _BasicSafetyMessage_H_ */
#include "asn_internal.h"