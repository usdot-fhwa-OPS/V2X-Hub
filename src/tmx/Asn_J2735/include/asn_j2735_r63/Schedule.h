/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "J2735_201603_ASN_CC.asn"
 * 	`asn1c -gen-PER -fcompound-names -fincludes-quoted`
 */

#ifndef	_Schedule_H_
#define	_Schedule_H_


#include "asn_application.h"

/* Including external dependencies */
#include "INTEGER.h"
#include "BOOLEAN.h"
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct RepeatParams;
struct DaySchedule;

/* Schedule */
typedef struct Schedule {
	INTEGER_t	 start;
	INTEGER_t	 end;
	struct Schedule__dow {
		A_SEQUENCE_OF(BOOLEAN_t) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} dow;
	struct Schedule__between {
		A_SEQUENCE_OF(struct DaySchedule) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *between;
	struct RepeatParams	*repeat;	/* OPTIONAL */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Schedule_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Schedule;
extern asn_SEQUENCE_specifics_t asn_SPC_Schedule_specs_1;
extern asn_TYPE_member_t asn_MBR_Schedule_1[5];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "RepeatParams.h"
#include "DaySchedule.h"

#endif	/* _Schedule_H_ */
#include "asn_internal.h"
