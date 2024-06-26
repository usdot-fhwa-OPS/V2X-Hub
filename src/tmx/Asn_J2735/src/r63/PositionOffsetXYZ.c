/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SDSM"
 * 	found in "J2735_201603_2023-06-22.asn"
 * 	`asn1c -fcompound-names `
 */

#include "PositionOffsetXYZ.h"

asn_TYPE_member_t asn_MBR_PositionOffsetXYZ_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct PositionOffsetXYZ, offsetX),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ObjectDistance,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"offsetX"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct PositionOffsetXYZ, offsetY),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ObjectDistance,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"offsetY"
		},
	{ ATF_POINTER, 1, offsetof(struct PositionOffsetXYZ, offsetZ),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ObjectDistance,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"offsetZ"
		},
};
static const int asn_MAP_PositionOffsetXYZ_oms_1[] = { 2 };
static const ber_tlv_tag_t asn_DEF_PositionOffsetXYZ_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_PositionOffsetXYZ_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* offsetX */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* offsetY */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* offsetZ */
};
asn_SEQUENCE_specifics_t asn_SPC_PositionOffsetXYZ_specs_1 = {
	sizeof(struct PositionOffsetXYZ),
	offsetof(struct PositionOffsetXYZ, _asn_ctx),
	asn_MAP_PositionOffsetXYZ_tag2el_1,
	3,	/* Count of tags in the map */
	asn_MAP_PositionOffsetXYZ_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_PositionOffsetXYZ = {
	"PositionOffsetXYZ",
	"PositionOffsetXYZ",
	&asn_OP_SEQUENCE,
	asn_DEF_PositionOffsetXYZ_tags_1,
	sizeof(asn_DEF_PositionOffsetXYZ_tags_1)
		/sizeof(asn_DEF_PositionOffsetXYZ_tags_1[0]), /* 1 */
	asn_DEF_PositionOffsetXYZ_tags_1,	/* Same as above */
	sizeof(asn_DEF_PositionOffsetXYZ_tags_1)
		/sizeof(asn_DEF_PositionOffsetXYZ_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_PositionOffsetXYZ_1,
	3,	/* Elements count */
	&asn_SPC_PositionOffsetXYZ_specs_1	/* Additional specs */
};

