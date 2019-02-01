/*************************************************************************************
*
*  map.h    : MAP Message Header File
*
*************************************************************************************/

#ifndef MAP_H_
#define MAP_H_

/* Constants */
#define map_maxstringlen 63
#define map_maxgeometries 10
#define map_maxbarriers 20
#define map_maxnodes 20
#define map_maxlanes 30
#define map_maxconnections 10

/* Lane Attribute Bit Flags */
#define LANE_ATTR_TWO_WAY_TRAVEL 					0x0001
#define LANE_ATTR_STRAIGHT_MANEUVER_PERMITTED 		0x0002
#define LANE_ATTR_LEFT_TURN_MANEUVER_PERMITTED 		0x0004
#define LANE_ATTR_RIGHT_TURN_MANEUVER_PERMITTED 	0x0008
#define LANE_ATTR_YIELD							 	0x0010
#define LANE_ATTR_NO_U_TURN						 	0x0020
#define LANE_ATTR_NO_TURN_ON_RED				 	0x0040
#define LANE_ATTR_NO_STOPPING					 	0x0080
#define LANE_ATTR_HOV_LANE						 	0x0100
#define LANE_ATTR_BUS_ONLY_LANE					 	0x0200
#define LANE_ATTR_BUS_AND_TAXI_ONLY_LANE		 	0x0400
#define LANE_ATTR_SHARED_TWO_WAY_LEFT_TURN_LANE	 	0x0800
#define LANE_ATTR_BIKE_LANE						 	0x1000

namespace MapPlugin {

/* MAP Enumerations */
enum map_type {intersection=1, roadway=2};
enum map_message_attributes {elevation=1, decimeter=2, geometric=4, navigational=8};
enum map_node_attributes {width=1, packed=2};
enum map_group_direction {approach=1, egress=2};
// DMC: Added additional lane types to support translation to R41 specification.
// This breaks compatibility with the older version.
// All types AFTER special are for R41.
enum map_lane_type {vehicle = 1, computed = 2, pedestrian = 3, special = 4,
	crosswalk = 10, bike = 11, sidewalk = 12, barrier = 13, striping = 14, trackedVehicle = 15, parking = 16};

/* MAP Structures */
struct map_referencelane
    {
    unsigned char lanenumber;
    signed short lateraloffset;
    signed short xoffset;
    signed short yoffset;
    };
struct map_connection
    {
    unsigned char lanenumber;
    unsigned char connectionID;
    unsigned char signalGroup;
    unsigned short maneuver;
    };
struct map_node
    {
    signed short eastern;
    signed short northern;
    signed short elevation;
    signed short width;
    };
struct map_lane
    {
    unsigned char number;
    unsigned char laneName[map_maxstringlen];
    unsigned char type;
    unsigned short attributes;
    unsigned short width;
    map_node node[map_maxnodes];
    map_referencelane referencelane;
    map_connection connection[map_maxconnections];
    };
struct map_group
    {
    unsigned short width;
    map_lane lane[map_maxlanes];
    };
struct map_barrier
    {
    unsigned short attributes;
    unsigned short width;
    map_node node[map_maxnodes];
    };       
struct map_referencepoint
    {
    signed int latitude;
    signed int longitude;
    signed int elevation;
    };
struct map_geometry
    {
    map_referencepoint refpoint;
    map_group approach;//ingress
    map_group egress;
    map_barrier barrier[map_maxbarriers];
    };

/*!
 * Map structure containing the GID information for an intersection
 * \ingroup MAPPlugin
 */

struct map
    {
	map_type mapType;
	unsigned char mapName[map_maxstringlen];
	//Deprecated.  Use MapType and MapId instead.
    unsigned long intersectionid;
    unsigned long mapid;
    unsigned char attributes;
    map_geometry geometry[map_maxgeometries];
    bblob payload;
    };   
    
/* MAP Function Prototypes */       
int map_initialize(map*);
int map_encode(map*, unsigned char);
int map_decode(map*);

} /* End namespace MapPlugin */
#endif
