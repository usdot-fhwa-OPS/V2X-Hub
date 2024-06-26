/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "J2735_201603_ASN_CC.asn"
 * 	`asn1c -gen-PER -fcompound-names -fincludes-quoted -fskeletons-copy`
 */

#include "PathNode.h"

static int
memb_x_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -32768 && value <= 32767)) {
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
memb_y_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -32768 && value <= 32767)) {
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
memb_z_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -32768 && value <= 32767)) {
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
memb_width_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	long value;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	value = *(const long *)sptr;
	
	if((value >= -128 && value <= 127)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_memb_x_constr_2 CC_NOTUSED = {
	{ 2, 0 }	/* (-32768..32767) */,
	-1};
static asn_per_constraints_t asn_PER_memb_x_constr_2 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 16,  16, -32768,  32767 }	/* (-32768..32767) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_y_constr_3 CC_NOTUSED = {
	{ 2, 0 }	/* (-32768..32767) */,
	-1};
static asn_per_constraints_t asn_PER_memb_y_constr_3 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 16,  16, -32768,  32767 }	/* (-32768..32767) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_z_constr_4 CC_NOTUSED = {
	{ 2, 0 }	/* (-32768..32767) */,
	-1};
static asn_per_constraints_t asn_PER_memb_z_constr_4 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 16,  16, -32768,  32767 }	/* (-32768..32767) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_width_constr_5 CC_NOTUSED = {
	{ 1, 0 }	/* (-128..127) */,
	-1};
static asn_per_constraints_t asn_PER_memb_width_constr_5 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 8,  8, -128,  127 }	/* (-128..127) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
asn_TYPE_member_t asn_MBR_PathNode_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PathNode, x),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_x_constr_2, &asn_PER_memb_x_constr_2,  memb_x_constraint_1 },
		0, 0, /* No default value */
		"x"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PathNode, y),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_y_constr_3, &asn_PER_memb_y_constr_3,  memb_y_constraint_1 },
		0, 0, /* No default value */
		"y"
		},
	{ ATF_POINTER, 2, offsetof(struct PathNode, z),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_z_constr_4, &asn_PER_memb_z_constr_4,  memb_z_constraint_1 },
		0, 0, /* No default value */
		"z"
		},
	{ ATF_POINTER, 1, offsetof(struct PathNode, width),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,
		{ &asn_OER_memb_width_constr_5, &asn_PER_memb_width_constr_5,  memb_width_constraint_1 },
		0, 0, /* No default value */
		"width"
		},
};
static const int asn_MAP_PathNode_oms_1[] = { 2, 3 };
static const ber_tlv_tag_t asn_DEF_PathNode_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PathNode_tag2el_1[] = {
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 1 }, /* x */
    { (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 1, -1, 0 }, /* y */
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 2, 0, 0 }, /* z */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 3, 0, 0 } /* width */
};
asn_SEQUENCE_specifics_t asn_SPC_PathNode_specs_1 = {
	sizeof(struct PathNode),
	offsetof(struct PathNode, _asn_ctx),
	asn_MAP_PathNode_tag2el_1,
	4,	/* Count of tags in the map */
	asn_MAP_PathNode_oms_1,	/* Optional members */
	2, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PathNode = {
	"PathNode",
	"PathNode",
	&asn_OP_SEQUENCE,
	asn_DEF_PathNode_tags_1,
	sizeof(asn_DEF_PathNode_tags_1)
		/sizeof(asn_DEF_PathNode_tags_1[0]), /* 1 */
	asn_DEF_PathNode_tags_1,	/* Same as above */
	sizeof(asn_DEF_PathNode_tags_1)
		/sizeof(asn_DEF_PathNode_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PathNode_1,
	4,	/* Elements count */
	&asn_SPC_PathNode_specs_1	/* Additional specs */
};

