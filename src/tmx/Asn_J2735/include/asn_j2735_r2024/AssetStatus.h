/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "J2540ITIS"
 * 	found in "J2540-2-J2540ITIS-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_AssetStatus_H_
#define	_AssetStatus_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeInteger.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AssetStatus {
	AssetStatus_unknown_status	= 10240,
	AssetStatus_ready_for_use	= 10241,
	AssetStatus_working_normally	= 10242,
	AssetStatus_working_autonomously	= 10243,
	AssetStatus_working_incorrectly	= 10244,
	AssetStatus_not_working	= 10245,
	AssetStatus_normal_maintenance	= 10246,
	AssetStatus_in_route_to_use	= 10247,
	AssetStatus_returning_from_use	= 10248,
	AssetStatus_out_of_service	= 10249,
	AssetStatus_off_duty	= 10250,
	AssetStatus_on_patrol	= 10251,
	AssetStatus_on_call	= 10252,
	AssetStatus_on_break	= 10253,
	AssetStatus_mandatory_time_off	= 10254,
	AssetStatus_low_on_fuel	= 10255,
	AssetStatus_low_on_water	= 10256,
	AssetStatus_low_charge	= 10257,
	AssetStatus_missing	= 10258
} e_AssetStatus;

/* AssetStatus */
typedef long	 AssetStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AssetStatus;
asn_struct_free_f AssetStatus_free;
asn_struct_print_f AssetStatus_print;
asn_constr_check_f AssetStatus_constraint;
ber_type_decoder_f AssetStatus_decode_ber;
der_type_encoder_f AssetStatus_encode_der;
xer_type_decoder_f AssetStatus_decode_xer;
xer_type_encoder_f AssetStatus_encode_xer;
oer_type_decoder_f AssetStatus_decode_oer;
oer_type_encoder_f AssetStatus_encode_oer;
per_type_decoder_f AssetStatus_decode_uper;
per_type_encoder_f AssetStatus_encode_uper;
per_type_decoder_f AssetStatus_decode_aper;
per_type_encoder_f AssetStatus_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _AssetStatus_H_ */
#include "asn_internal.h"