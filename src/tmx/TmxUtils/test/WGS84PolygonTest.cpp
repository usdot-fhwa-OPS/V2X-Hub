/*
 * WGS84PolygonTest.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: ivp
 */


#include <gtest/gtest.h>
#include "WGS84Polygon.h"
using namespace std;
using namespace tmx::utils;



namespace unit_test {

class WGS84PolygonTest: public testing::Test {
protected:
	WGS84PolygonTest() {

	}

	virtual ~WGS84PolygonTest() {
	}

	WGS84Polygon _poly;
};

TEST_F(WGS84PolygonTest, IsPointInsidePoly) {

bool pointInside = true;

WGS84Point A = { 40.910957,-81.597088};
WGS84Point B = {  40.910826,-81.596895};
WGS84Point C = {  40.910707,-81.597180};
WGS84Point D = {   40.910719,-81.596894};

vector<WGS84Point> vec;
vec.push_back(A);
vec.push_back(B);
vec.push_back(C);
vec.push_back(D);

WGS84Point testpoint = { 40.910740, -81.596935};//inside box
pointInside=_poly.IsPointInsidePoly(testpoint,vec);
EXPECT_TRUE(pointInside);


testpoint = {  40.910694, -81.596886};//outside box
pointInside=_poly.IsPointInsidePoly(testpoint,vec);
EXPECT_FALSE(pointInside);
}
}  // namespace





