/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeDataReport"
 * 	found in "J2945-C-ProbeDataReport-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_RptWiperStatus_H_
#define	_RptWiperStatus_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RptWiperStatus {
	RptWiperStatus_off	= 0,
	RptWiperStatus_low	= 1,
	RptWiperStatus_medium	= 2,
	RptWiperStatus_high	= 3
	/*
	 * Enumeration is extensible
	 */
} e_RptWiperStatus;

/* RptWiperStatus */
typedef long	 RptWiperStatus_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_RptWiperStatus_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_RptWiperStatus;
extern const asn_INTEGER_specifics_t asn_SPC_RptWiperStatus_specs_1;
asn_struct_free_f RptWiperStatus_free;
asn_struct_print_f RptWiperStatus_print;
asn_constr_check_f RptWiperStatus_constraint;
ber_type_decoder_f RptWiperStatus_decode_ber;
der_type_encoder_f RptWiperStatus_encode_der;
xer_type_decoder_f RptWiperStatus_decode_xer;
xer_type_encoder_f RptWiperStatus_encode_xer;
oer_type_decoder_f RptWiperStatus_decode_oer;
oer_type_encoder_f RptWiperStatus_encode_oer;
per_type_decoder_f RptWiperStatus_decode_uper;
per_type_encoder_f RptWiperStatus_encode_uper;
per_type_decoder_f RptWiperStatus_decode_aper;
per_type_encoder_f RptWiperStatus_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _RptWiperStatus_H_ */
#include "asn_internal.h"