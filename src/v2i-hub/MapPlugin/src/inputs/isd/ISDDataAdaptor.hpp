/*
 * ISDDataAdaptor.hpp
 *
 *  Created on: May 31, 2017
 *      Author: gmb
 */

#ifndef INPUTS_ISD_ISDDATAADAPTOR_HPP_
#define INPUTS_ISD_ISDDATAADAPTOR_HPP_

#include <tmx/j2735_messages/MapDataMessage.hpp>
#include <MinuteOfTheYear.h>
#include <bitset>

#if SAEJ2735_SPEC < 63
#define NO_NODE NodeList2_PR_NOTHING
#define DEFINED_NODE NodeList2_PR_nodes
#define COMPUTED_NODE NodeList2_PR_computed
#define ENUM_NAME(X) NodeOffsetPoint_PR_node_ ## X
#else
#define NO_NODE NodeListXY_PR_NOTHING
#define DEFINED_NODE NodeListXY_PR_nodes
#define COMPUTED_NODE NodeListXY_PR_computed
#define ENUM_NAME(X) NodeOffsetPointXY_PR_node_ ## X
#endif
#if SAEJ2735_SPEC < 2020
typedef DSRC_Elevation_t Elevation2_t;
#else
typedef Common_Elevation_t Elevation2_t;
#endif
typedef NodeXY Node;
typedef NodeAttributeSetXY NodeAttributeSet;


#define attribute(X, Y, D) ro_attribute(this->msg, battelle::attributes::standard_attribute<Y>, X, Y, get_, D)
#define marshalfuncs(X) \
	static tmx::message_tree_type to_tree(X m) \
 	{ return tmx::message::to_tree(static_cast<tmx::message>(m)); } \
 	static X from_tree(tmx::message_tree_type t) \
 	{ return MapPlugin::CreateAdaptor<X>(tmx::message::from_tree(t).get_container().get_storage().get_tree()); }

namespace MapPlugin {

template <class Adaptor>
Adaptor CreateAdaptor(const tmx::message_tree_type &tree)
{
	Adaptor adaptor;
	adaptor.set_contents(tree);
	return adaptor;
}

template <class Adaptor>
Adaptor CreateAdaptor(boost::optional<tmx::message_tree_type &> tree)
{
	if (tree)
		return CreateAdaptor<Adaptor>(tree.get());
	return Adaptor();
}

class LaneNodeAdaptor: public tmx::message {
public:
	LaneNodeAdaptor(): tmx::message() {}
	virtual ~LaneNodeAdaptor() {}

	marshalfuncs(LaneNodeAdaptor)

	attribute(long, nodeNumber, -1);
	attribute(double, nodeLat, 0.0);
	attribute(double, nodeLong, 0.0);
	attribute(double, nodeElev, -1000.0);
	attribute(long, laneWidthDelta, 0);
};

class ConnectionAdaptor: public tmx::message {
public:
	ConnectionAdaptor(): tmx::message() {}
	virtual ~ConnectionAdaptor() {}

	marshalfuncs(ConnectionAdaptor)

	attribute(std::string, fromLane, "");
	attribute(std::string, toLane, "");
	attribute(std::string, signal_id, "");

	std::bitset<12> get_maneuvers() {
		std::bitset<12> maneuvers;
		for (auto elem : this->as_tree().get().get_child("maneuvers."))
			maneuvers[elem.second.get_value<size_t>()] = true;
		return maneuvers;
	}
};

class DrivingLaneAdaptor: public tmx::message {
public:
	DrivingLaneAdaptor(): tmx::message() {}
	virtual ~DrivingLaneAdaptor() {}

	marshalfuncs(DrivingLaneAdaptor)

	std::vector<ConnectionAdaptor> get_connections() {
		return this->template get_array<ConnectionAdaptor>("connections");
	}

	std::vector<LaneNodeAdaptor> get_laneNodes() {
		return this->template get_array<LaneNodeAdaptor>("laneNodes");
	}

	attribute(std::string, laneID, "");
	attribute(std::string, laneType, "");

	std::bitset<16> get_typeAttributes() {
		std::bitset<16> typeAttrs;
		for (auto elem : this->as_tree().get().get_child("typeAttributes."))
			typeAttrs[elem.second.get_value<size_t>()] = true;
		return typeAttrs;
	}

	std::bitset<10> get_sharedWith() {
		std::bitset<10> sharedWith;
		for (auto elem : this->as_tree().get().get_child("sharedWith."))
			sharedWith[elem.second.get_value<size_t>()] = true;
		return sharedWith;
	}

	std::bitset<12> get_laneManeuvers() {
		std::bitset<12> maneuvers;
		for (auto elem : this->as_tree().get().get_child("laneManeuvers."))
			maneuvers[elem.second.get_value<size_t>()] = true;
		return maneuvers;
	}
};

class LaneListAdaptor: public tmx::message {
public:
	LaneListAdaptor(): tmx::message() {}
	virtual ~LaneListAdaptor() {}

	std::vector<DrivingLaneAdaptor> get_drivingLanes() {
		return this->template get_array<DrivingLaneAdaptor>("drivingLanes");
	}

	std::vector<DrivingLaneAdaptor> get_crosswalkLanes() {
		return this->template get_array<DrivingLaneAdaptor>("crosswalkLanes");
	}

	marshalfuncs(LaneListAdaptor);

	attribute(std::string, approachType, "");
	attribute(std::string, approachID, "");
};

class IntersectionGeometryAdaptor: public tmx::message {
public:
	IntersectionGeometryAdaptor(): tmx::message() {}
	virtual ~IntersectionGeometryAdaptor() {}

	std::vector<LaneListAdaptor> get_lanes() {
		return this->template get_array<LaneListAdaptor>("laneList.approach");
	}

	marshalfuncs(IntersectionGeometryAdaptor)
private:
    struct ReferencePointCreator
	{
		typedef tmx::message_path_type path_type;

		path_type operator()(path_type attr_path)
		{
			path_type p("referencePoint");
			p /= attr_path;
			return p;
		}
	};

    template <typename T>
    using reference_point_attribute =
    		battelle::attributes::standard_2level_attribute<T, tmx::message_container_type::storage_type, ReferencePointCreator>;

 	ro_attribute(this->msg, reference_point_attribute<descriptiveIntersctionName>, std::string, descriptiveIntersctionName, get_, "");
    ro_attribute(this->msg, reference_point_attribute<layerID>, LayerID_t, layerID, get_, 0);
    ro_attribute(this->msg, reference_point_attribute<intersectionID>, IntersectionID_t, intersectionID, get_, 0);
	#if SAEJ2735_SPEC < 63
    ro_attribute(this->msg, reference_point_attribute<msgCount>, MsgCount_t, msgCount, get_, 0);
	#elif SAEJ2735_SPEC < 2020
    ro_attribute(this->msg, reference_point_attribute<msgCount>, DSRC_MsgCount_t, msgCount, get_, 0);
	#else
	ro_attribute(this->msg, reference_point_attribute<msgCount>, Common_MsgCount_t, msgCount, get_, 0);
	#endif
    ro_attribute(this->msg, reference_point_attribute<masterLaneWidth>, LaneWidth_t, masterLaneWidth, get_, -1L);
    ro_attribute(this->msg, reference_point_attribute<referenceLat>, double, referenceLat, get_, 0.0);
    ro_attribute(this->msg, reference_point_attribute<referenceLon>, double, referenceLon, get_, 0.0);
    ro_attribute(this->msg, reference_point_attribute<referenceElevation>, double, referenceElevation, get_, -1.0);
	ro_attribute(this->msg, reference_point_attribute<intersectionType>, long, intersectionType, get_, 0);
};

class ISDDataAdaptor: public tmx::message {
public:
	ISDDataAdaptor(): tmx::message() {}
	virtual ~ISDDataAdaptor() {}

	IntersectionGeometryAdaptor get_IntersectionGeometry() {
		return CreateAdaptor<IntersectionGeometryAdaptor>(this->as_tree("mapData.intersectionGeometry"));
	}

private:
	struct MapDataCreator
	{
		typedef tmx::message_path_type path_type;

		path_type operator()(path_type attr_path)
		{
			path_type p("mapData");
			p /= attr_path;
			return p;
		}
	};

	template <typename T>
	using map_data_attribute =
			battelle::attributes::standard_2level_attribute<T, tmx::message_container_type::storage_type, MapDataCreator>;

	ro_attribute(this->msg, map_data_attribute<minuteOfTheYear>, MinuteOfTheYear_t, minuteOfTheYear, get_, 0);
	ro_attribute(this->msg, map_data_attribute<layerType>, std::string, layerType, get_, "");
};

} /* namespace MapPlugin */

#endif /* INPUTS_ISD_ISDDATAADAPTOR_HPP_ */
