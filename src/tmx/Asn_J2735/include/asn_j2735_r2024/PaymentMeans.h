/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "EfcDataDictionary"
 * 	found in "ISO17573-EfcDataDictionary.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_PaymentMeans_H_
#define	_PaymentMeans_H_


#include "asn_application.h"

/* Including external dependencies */
#include "PersonalAccountNumber.h"
#include "DateCompact.h"
#include "OCTET_STRING.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PaymentMeans */
typedef struct PaymentMeans {
	PersonalAccountNumber_t	 personalAccountNumber;
	DateCompact_t	 paymentMeansExpiryDate;
	OCTET_STRING_t	 pamentMeansUsageControl;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PaymentMeans_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PaymentMeans;

#ifdef __cplusplus
}
#endif

#endif	/* _PaymentMeans_H_ */
#include "asn_internal.h"