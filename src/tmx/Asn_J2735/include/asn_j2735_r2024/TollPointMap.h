/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TollAdvertisementMessage"
 * 	found in "J3217-TollAdvertisementMsg-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_TollPointMap_H_
#define	_TollPointMap_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeInteger.h"
#include "Position3D.h"
#include "RegulatorySpeedLimit.h"
#include "LaneWidth.h"
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct GenericLane;

/* TollPointMap */
typedef struct TollPointMap {
	long	 revisionNum;
	Position3D_t	 referencePoint;
	RegulatorySpeedLimit_t	 speedLimit;
	LaneWidth_t	 laneWidth;
	struct TollPointMap__approachLanesMap {
		A_SEQUENCE_OF(struct GenericLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} approachLanesMap;
	struct TollPointMap__tollZoneLanesMap {
		A_SEQUENCE_OF(struct GenericLane) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} tollZoneLanesMap;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TollPointMap_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TollPointMap;
extern asn_SEQUENCE_specifics_t asn_SPC_TollPointMap_specs_1;
extern asn_TYPE_member_t asn_MBR_TollPointMap_1[6];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "GenericLane.h"

#endif	/* _TollPointMap_H_ */
#include "asn_internal.h"