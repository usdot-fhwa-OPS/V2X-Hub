/*
 * WGS84Polygon.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: ivp
 */

#include "WGS84Polygon.h"

using namespace std;

namespace tmx {
namespace utils {

WGS84Polygon::~WGS84Polygon() {
	// TODO Auto-generated destructor stub
}

WGS84Polygon::WGS84Polygon() {
	npoints = 4;
//	mWGS84Points = new WGS84Point[4];
}

//WGS84Polygon:: WGS84Polygon(WGS84Point[] points)
//{
//	npoints = points.length;
//
//	mWGS84Points = new WGS84Point[npoints];
//
//	for(int i =0; i<points.length;i++)
//	{
//		mWGS84Points[i] = points[i];
//	}
//}

bool WGS84Polygon::IsPointInsidePoly(WGS84Point pointToTest,WGS84Point A, WGS84Point B, WGS84Point C, WGS84Point D) {
	vector<WGS84Point> vec;
	vec.push_back(A);
	vec.push_back(B);
	vec.push_back(C);
	vec.push_back(D);

	return IsPointInsidePoly(pointToTest,vec);
}

/**
 * Determines if the specified coordinates are inside this
 * <code>Polygon</code>. For the definition of <i>insideness</i>.
 *
 * @param x
 *            the specified x coordinate
 * @param y
 *            the specified y coordinate
 * @return <code>true</code> if the <code>Polygon</code> contains the
 *         specified coordinates; <code>false</code> otherwise.
 */
bool WGS84Polygon::IsPointInsidePoly(WGS84Point pointToTest,std::vector<WGS84Point> polyPoints) {

	double x = pointToTest.Longitude;
	double y = pointToTest.Latitude;

	if (polyPoints.size() <= 2) {//validate min number of points for polygon
		return false;
	}
	int hits = 0;

	//Get the xy of the last point on the polygon.
//	double lastx = mWGS84Points[npoints - 1].Longitude;
//	double lasty = mWGS84Points[npoints - 1].Latitude;
	double lastx = polyPoints.back().Longitude;
	double lasty = polyPoints.back().Latitude;
	double curx, cury;

	// Walk the edges of the polygon
	//std::vector<WGS84Point>::iterator pIter = polyPoints.begin();
	for (std::vector<WGS84Point>::iterator pIter = polyPoints.begin();
			pIter != polyPoints.end(); lastx = curx, lasty = cury,++pIter) {
	//for (int i = 0; i < npoints; lastx = curx, lasty = cury, i++) {
		curx = pIter->Longitude;
		cury = pIter->Latitude;

		if (cury == lasty) {
			continue;
		}

		double leftx;
		if (curx < lastx) {
			if (x >= lastx) {
				continue;
			}
			leftx = curx;
		} else {
			if (x >= curx) {
				continue;
			}
			leftx = lastx;
		}

		double test1, test2;
		if (cury < lasty) {
			if (y < cury || y >= lasty) {
				continue;
			}
			if (x < leftx) {
				hits++;
				continue;
			}
			test1 = x - curx;
			test2 = y - cury;
		} else {
			if (y < lasty || y >= cury) {
				continue;
			}
			if (x < leftx) {
				hits++;
				continue;
			}
			test1 = x - lastx;
			test2 = y - lasty;
		}

		if (test1 < (test2 / (lasty - cury) * (lastx - curx))) {
			hits++;
		}
	}

	return ((hits & 1) != 0);
}

}} // namespace tmx::utils
