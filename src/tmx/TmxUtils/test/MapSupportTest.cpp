//============================================================================
// Name        : TestMapSupport.cpp
// Description : Unit tests for MapSupport.
//============================================================================

#include <gtest/gtest.h>
#include <MapSupport.h>
#include <WGS84Point.h>
#include <ParsedMap.h>

using namespace std;
using namespace tmx::utils;

namespace unit_test {

class MapSupportTest : public testing::Test
{
protected:
	MapSupportTest()
	{
		_mapLane.LaneWidthMeters = 350;
		_mapLane.Nodes.emplace_back(42.5011, -83.2848);
		_mapLane.Nodes.emplace_back(42.5012, -83.284);
		_mapLane.Nodes.emplace_back(42.5012, -83.2834);


		_map.ReferencePoint = {39.9876814,-83.0207827};

		//Add region 3 lanes:

		//3 nodes
		_mapLane.LaneWidthMeters = 350;
			_mapLane.Nodes.emplace_back(42.5011, -83.2848);
			_mapLane.Nodes.emplace_back(42.5012, -83.284);
			_mapLane.Nodes.emplace_back(42.5012, -83.2834);


			_map.ReferencePoint = {39.9876814,-83.0207827};

			//Add region 3 lanes:

			//3 nodes
			LaneNode lane31node0 = LaneNode(39.987558818127553,-83.020802631749078);
			LaneNode lane31node1 = LaneNode(39.98731545705725,-83.02081318385153);
			LaneNode lane31node2 = LaneNode(39.98721000059345,-83.020820218586508);

			MapLane lane31;

			//Add nodes to lane.
			lane31.Nodes.push_back(lane31node0);
			lane31.Nodes.push_back(lane31node1);
			lane31.Nodes.push_back(lane31node2);
			lane31.LaneWidthMeters = 3.5;
			lane31.LaneNumber = 31;
			lane31.Type = LaneType::Vehicle;
			lane31.Direction = DirectionalUse::Ingress_Vehicle_Computed;
			//add lane to map
			_map.Lanes.push_back(lane31);



			LaneNode lane32node0 = LaneNode(39.987558818127553,-83.020769803044786);
			LaneNode lane32node1 = LaneNode(39.98731545705725,-83.020780355264193);
			LaneNode lane32node2 = LaneNode(39.98721000059345,-83.020787390049847);

			MapLane lane32;
			//Add nodes to lane.
			lane32.Nodes.push_back(lane32node0);
			lane32.Nodes.push_back(lane32node1);
			lane32.Nodes.push_back(lane32node2);
			lane32.LaneNumber = 32;
			lane32.LaneWidthMeters = 3.5;
			lane32.Type = LaneType::Computed;
			lane32.Direction =DirectionalUse::Ingress_Vehicle_Computed;
			//add lane to map
			_map.Lanes.push_back(lane32);

			LaneNode lane33node0 = LaneNode(39.987558818127553,-83.020843667629435);
			LaneNode lane33node1 = LaneNode(39.98731545705725,-83.020854219585701);
			LaneNode lane33node2 = LaneNode(39.98721000059345,-83.020861254257326);

			MapLane lane33;
			//Add nodes to lane.
			lane33.Nodes.push_back(lane33node0);
			lane33.Nodes.push_back(lane33node1);
			lane33.Nodes.push_back(lane33node2);
			lane33.LaneNumber = 33;
			lane33.LaneWidthMeters = 3.5;
			lane33.Type = LaneType::Egress;
			lane33.Direction = DirectionalUse::Egress_Computed;
			//add lane to map
			_map.Lanes.push_back(lane33);

			_map.MinLat = 39;
			_map.MaxLat = 40;
			_map.MinLong = -84;
			_map.MaxLong = -83;
	}

	virtual ~MapSupportTest()
	{
	}

	MapSupport _mapSupport;
	ParsedMap _map;
	MapLane _mapLane;
};
TEST_F(MapSupportTest, IsVehicleLane)
{
	bool r = _mapSupport.IsVehicleLane(31, _map);
	EXPECT_TRUE(r);
	r = _mapSupport.IsVehicleLane(32, _map);
		EXPECT_TRUE(r);
		r = _mapSupport.IsVehicleLane(33, _map);
			EXPECT_TRUE(r);
			r = _mapSupport.IsVehicleLane(25, _map);
				EXPECT_FALSE(r);
}
TEST_F(MapSupportTest, PointIsInLane)
{
	MapMatchResult match;

	match = _mapSupport.PointIsInLane(_mapLane, WGS84Point(42.5011, -83.2848));
	EXPECT_TRUE(match.IsInLane);

	match = _mapSupport.PointIsInLane(_mapLane, WGS84Point(41.5011, -83.2848));
	EXPECT_FALSE(match.IsInLane);



	WGS84Point lane31segmentOutside={ 39.987138,-83.020821};
				MapMatchResult foundLane31Out = _mapSupport.FindVehicleLaneForPoint(lane31segmentOutside,_map );
				//ASSERT_EQ(33, foundLane33.LaneNumber);
				ASSERT_EQ(false, foundLane31Out.IsInLane);
				//ASSERT_EQ(true, foundLane33.IsEgress);
				//ASSERT_EQ(1, foundLane33.LaneSegment);

	MapMatchResult foundLane;

		WGS84Point lane31segment1={39.987439,-83.020800};
		foundLane = _mapSupport.FindVehicleLaneForPoint(lane31segment1,_map );
		ASSERT_EQ(31, foundLane.LaneNumber);
		ASSERT_EQ(true, foundLane.IsInLane);
		ASSERT_EQ(false, foundLane.IsEgress);
		ASSERT_EQ(1, foundLane.LaneSegment);

		WGS84Point lane32segment2={ 39.987260,-83.020769};
		MapMatchResult foundLane32 = _mapSupport.FindVehicleLaneForPoint(lane32segment2,_map );
		ASSERT_EQ(32, foundLane32.LaneNumber);
		ASSERT_EQ(true, foundLane32.IsInLane);
		ASSERT_EQ(false, foundLane32.IsEgress);
		ASSERT_EQ(2, foundLane32.LaneSegment);

		WGS84Point lane33segment1={  39.987500,-83.020840};
		MapMatchResult foundLane33 = _mapSupport.FindVehicleLaneForPoint(lane33segment1,_map );
		ASSERT_EQ(33, foundLane33.LaneNumber);
		ASSERT_EQ(true, foundLane33.IsInLane);
		ASSERT_EQ(true, foundLane33.IsEgress);
		ASSERT_EQ(1, foundLane33.LaneSegment);



}


TEST_F(MapSupportTest, IsInCenterOfIntersection)
{
	bool r;
	WGS84Point pointOutside30m = WGS84Point{39.987684, -83.021218};
	WGS84Point pointInCenter = WGS84Point{39.987616, -83.020716};



	r = _mapSupport.IsInCenterOfIntersection(pointInCenter,_map);
	EXPECT_TRUE(r);

	r = _mapSupport.IsInCenterOfIntersection(pointOutside30m,_map);
	EXPECT_FALSE(r);

}
TEST_F(MapSupportTest, IsInBox)
{
	ParsedMap nycmap;
	nycmap.MinLat = 40.669101761;
	nycmap.MaxLat = 40.6701076539;
	nycmap.MinLong = -73.8341055446;
	nycmap.MaxLong = -73.8329543434;

	 //out below
	  bool r = _mapSupport.IsPointOnMapUsa(WGS84Point{  40.668871, -73.833357},nycmap);
	 	EXPECT_FALSE(r);
//out left
	 	r = _mapSupport.IsPointOnMapUsa(WGS84Point{  40.669277, -73.834291},nycmap);
	 	EXPECT_FALSE(r);
	//lower left, in
	r = _mapSupport.IsPointOnMapUsa(WGS84Point { 40.669224, -73.833841 }, nycmap);
	EXPECT_TRUE(r);
//lower right, in
	r = _mapSupport.IsPointOnMapUsa(WGS84Point { 40.669434, -73.833130 }, nycmap);
	EXPECT_TRUE(r);

	//upper left, in
	r = _mapSupport.IsPointOnMapUsa(WGS84Point {  40.669826, -73.834038 }, nycmap);
	EXPECT_TRUE(r);
//upper right, in
	r = _mapSupport.IsPointOnMapUsa(WGS84Point {  40.670023, -73.833345 }, nycmap);
	EXPECT_TRUE(r);

	 //out above
		  r = _mapSupport.IsPointOnMapUsa(WGS84Point{ 40.670202, -73.83356},nycmap);
		 	EXPECT_FALSE(r);
	//out right
		 	r = _mapSupport.IsPointOnMapUsa(WGS84Point{  40.669930, -73.832846},nycmap);
		 	EXPECT_FALSE(r);



}

}  // namespace
