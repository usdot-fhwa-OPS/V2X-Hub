/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeDataConfig"
 * 	found in "J2945-C-ProbeDataConfig-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_CfgAveragedAndSummaryTriggers_H_
#define	_CfgAveragedAndSummaryTriggers_H_


#include "asn_application.h"

/* Including external dependencies */
#include "CfgAveragedTriggers.h"
#include "CfgSummaryTriggers.h"
#include "constr_CHOICE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum CfgAveragedAndSummaryTriggers_PR {
	CfgAveragedAndSummaryTriggers_PR_NOTHING,	/* No components present */
	CfgAveragedAndSummaryTriggers_PR_averagedTriggers,
	CfgAveragedAndSummaryTriggers_PR_summaryTriggers
	/* Extensions may appear below */
	
} CfgAveragedAndSummaryTriggers_PR;

/* CfgAveragedAndSummaryTriggers */
typedef struct CfgAveragedAndSummaryTriggers {
	CfgAveragedAndSummaryTriggers_PR present;
	union CfgAveragedAndSummaryTriggers_u {
		CfgAveragedTriggers_t	 averagedTriggers;
		CfgSummaryTriggers_t	 summaryTriggers;
		/*
		 * This type is extensible,
		 * possible extensions are below.
		 */
	} choice;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CfgAveragedAndSummaryTriggers_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CfgAveragedAndSummaryTriggers;
extern asn_CHOICE_specifics_t asn_SPC_CfgAveragedAndSummaryTriggers_specs_1;
extern asn_TYPE_member_t asn_MBR_CfgAveragedAndSummaryTriggers_1[2];
extern asn_per_constraints_t asn_PER_type_CfgAveragedAndSummaryTriggers_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _CfgAveragedAndSummaryTriggers_H_ */
#include "asn_internal.h"