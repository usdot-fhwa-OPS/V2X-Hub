/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "J2540ITIS"
 * 	found in "J2540-2-J2540ITIS-2024-rel-v1.1.asn"
 * 	`asn1c -pdu=MessageFrame -fcompound-names -fincludes-quoted -no-gen-JER`
 */

#ifndef	_RestrictionClass_H_
#define	_RestrictionClass_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeInteger.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RestrictionClass {
	RestrictionClass_restrictions	= 2561,
	RestrictionClass_ramp_restrictions	= 2562,
	RestrictionClass_truck_restriction	= 2563,
	RestrictionClass_speed_restriction	= 2564,
	RestrictionClass_noise_restriction	= 2565,
	RestrictionClass_traffic_regulations_have_been_changed	= 2566,
	RestrictionClass_local_access_only	= 2567,
	RestrictionClass_no_trailers	= 2568,
	RestrictionClass_no_high_profile_vehicles	= 2569,
	RestrictionClass_hazardous_materials_truck_restriction	= 2570,
	RestrictionClass_no_through_traffic	= 2571,
	RestrictionClass_no_motor_vehicles	= 2572,
	RestrictionClass_width_limit	= 2573,
	RestrictionClass_height_limit	= 2574,
	RestrictionClass_length_limit	= 2575,
	RestrictionClass_axle_load_limit	= 2576,
	RestrictionClass_gross_weight_limit	= 2577,
	RestrictionClass_axle_count_limit	= 2578,
	RestrictionClass_carpool_lane_available	= 2579,
	RestrictionClass_carpool_restrictions_changed	= 2580,
	RestrictionClass_hOV_2_no_single_occupant_vehicles	= 2581,
	RestrictionClass_hOV_3_no_vehicles_with_less_than_three_occupants	= 2582,
	RestrictionClass_bus_lane_available_for_all_vehicles	= 2583,
	RestrictionClass_truck_lane_available_for_all_vehicles	= 2584,
	RestrictionClass_permits_call_in_basis	= 2585,
	RestrictionClass_permits_temporarily_closed	= 2586,
	RestrictionClass_permits_closed	= 2587,
	RestrictionClass_road_use_permits_required	= 2588,
	RestrictionClass_permits_open	= 2675,
	RestrictionClass_restrictions_for_high_profile_vehicles_lifted	= 2676,
	RestrictionClass_width_limit_lifted	= 2677,
	RestrictionClass_height_limit_lifted	= 2678,
	RestrictionClass_length_limit_lifted	= 2679,
	RestrictionClass_axle_load_limit_lifted	= 2680,
	RestrictionClass_weight_limit_lifted	= 2681,
	RestrictionClass_axle_count_limit_lifted	= 2682,
	RestrictionClass_carpool_restrictions_lifted	= 2683,
	RestrictionClass_lane_restrictions_lifted	= 2684,
	RestrictionClass_ramp_restrictions_lifted	= 2685,
	RestrictionClass_motor_vehicle_restrictions_lifted	= 2686,
	RestrictionClass_restrictions_lifted	= 2687
} e_RestrictionClass;

/* RestrictionClass */
typedef long	 RestrictionClass_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_RestrictionClass_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_RestrictionClass;
asn_struct_free_f RestrictionClass_free;
asn_struct_print_f RestrictionClass_print;
asn_constr_check_f RestrictionClass_constraint;
ber_type_decoder_f RestrictionClass_decode_ber;
der_type_encoder_f RestrictionClass_encode_der;
xer_type_decoder_f RestrictionClass_decode_xer;
xer_type_encoder_f RestrictionClass_encode_xer;
oer_type_decoder_f RestrictionClass_decode_oer;
oer_type_encoder_f RestrictionClass_encode_oer;
per_type_decoder_f RestrictionClass_decode_uper;
per_type_encoder_f RestrictionClass_encode_uper;
per_type_decoder_f RestrictionClass_decode_aper;
per_type_encoder_f RestrictionClass_encode_aper;

#ifdef __cplusplus
}
#endif

#endif	/* _RestrictionClass_H_ */
#include "asn_internal.h"