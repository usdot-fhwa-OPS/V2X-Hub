/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SDSM"
 * 	found in "J2735_201603_2023-06-22.asn"
 * 	`asn1c -fcompound-names `
 */

#ifndef	_ObjectType_H_
#define	_ObjectType_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum ObjectType {
	ObjectType_unknown	= 0,
	ObjectType_vehicle	= 1,
	ObjectType_vru	= 2,
	ObjectType_animal	= 3
	/*
	 * Enumeration is extensible
	 */
} e_ObjectType;

/* ObjectType */
typedef long	 ObjectType_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_ObjectType_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_ObjectType;
extern const asn_INTEGER_specifics_t asn_SPC_ObjectType_specs_1;
asn_struct_free_f ObjectType_free;
asn_struct_print_f ObjectType_print;
asn_constr_check_f ObjectType_constraint;
ber_type_decoder_f ObjectType_decode_ber;
der_type_encoder_f ObjectType_encode_der;
xer_type_decoder_f ObjectType_decode_xer;
xer_type_encoder_f ObjectType_encode_xer;
oer_type_decoder_f ObjectType_decode_oer;
oer_type_encoder_f ObjectType_encode_oer;
per_type_decoder_f ObjectType_decode_uper;
per_type_encoder_f ObjectType_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _ObjectType_H_ */
#include <asn_internal.h>
