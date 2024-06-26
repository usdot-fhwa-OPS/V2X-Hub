/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "J2735_201603_ASN_CC.asn"
 * 	`asn1c -gen-PER -fcompound-names -fincludes-quoted -fskeletons-copy`
 */

#include "DailySchedule.h"

static int
memb_begin_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 1439)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_duration_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= 0 && value <= 1439)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_memb_begin_constr_2 CC_NOTUSED = {
	{ 2, 1 }	/* (0..1439) */,
	-1};
static asn_per_constraints_t asn_PER_memb_begin_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 11,  11,  0,  1439 }	/* (0..1439) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_duration_constr_3 CC_NOTUSED = {
	{ 2, 1 }	/* (0..1439) */,
	-1};
static asn_per_constraints_t asn_PER_memb_duration_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 11,  11,  0,  1439 }	/* (0..1439) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_DailySchedule_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct DailySchedule, begin),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_begin_constr_2, &asn_PER_memb_begin_constr_2,  memb_begin_constraint_1 },
		0, 0, /* No default value */
		"begin"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct DailySchedule, duration),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_duration_constr_3, &asn_PER_memb_duration_constr_3,  memb_duration_constraint_1 },
		0, 0, /* No default value */
		"duration"
		},
};
static const ber_tlv_tag_t asn_DEF_DailySchedule_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_DailySchedule_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* begin */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* duration */
};
asn_SEQUENCE_specifics_t asn_SPC_DailySchedule_specs_1 = {
	sizeof(struct DailySchedule),
	offsetof(struct DailySchedule, _asn_ctx),
	asn_MAP_DailySchedule_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_DailySchedule = {
	"DailySchedule",
	"DailySchedule",
	&asn_OP_SEQUENCE,
	asn_DEF_DailySchedule_tags_1,
	sizeof(asn_DEF_DailySchedule_tags_1)
		/sizeof(asn_DEF_DailySchedule_tags_1[0]), /* 1 */
	asn_DEF_DailySchedule_tags_1,	/* Same as above */
	sizeof(asn_DEF_DailySchedule_tags_1)
		/sizeof(asn_DEF_DailySchedule_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_DailySchedule_1,
	2,	/* Elements count */
	&asn_SPC_DailySchedule_specs_1	/* Additional specs */
};

