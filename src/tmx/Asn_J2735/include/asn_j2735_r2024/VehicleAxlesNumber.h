/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "EfcDataDictionary"
 * 	found in "ISO17573-EfcDataDictionary.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_VehicleAxlesNumber_H_
#define	_VehicleAxlesNumber_H_


#include "asn_application.h"

/* Including external dependencies */
#include "TyreConfiguration.h"
#include "NumberOfAxles.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* VehicleAxlesNumber */
typedef struct VehicleAxlesNumber {
	TyreConfiguration_t	 tyreConfiguration;
	NumberOfAxles_t	 numberOfAxles;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleAxlesNumber_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VehicleAxlesNumber;
extern asn_SEQUENCE_specifics_t asn_SPC_VehicleAxlesNumber_specs_1;
extern asn_TYPE_member_t asn_MBR_VehicleAxlesNumber_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _VehicleAxlesNumber_H_ */
#include "asn_internal.h"