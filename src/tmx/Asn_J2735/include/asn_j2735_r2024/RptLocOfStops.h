/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeDataReport"
 * 	found in "J2945-C-ProbeDataReport-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_RptLocOfStops_H_
#define	_RptLocOfStops_H_


#include "asn_application.h"

/* Including external dependencies */
#include "Position3D.h"
#include "PositionalAccuracy.h"
#include "NativeInteger.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* RptLocOfStops */
typedef struct RptLocOfStops {
	Position3D_t	 locationOfStop;
	PositionalAccuracy_t	 locAccOfStop;
	long	 durationOfStop;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RptLocOfStops_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RptLocOfStops;
extern asn_SEQUENCE_specifics_t asn_SPC_RptLocOfStops_specs_1;
extern asn_TYPE_member_t asn_MBR_RptLocOfStops_1[3];

#ifdef __cplusplus
}
#endif

#endif	/* _RptLocOfStops_H_ */
#include "asn_internal.h"