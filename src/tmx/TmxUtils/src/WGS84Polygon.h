/*
 * WGS84Polygon.h
 *
 *  Created on: Oct 17, 2016
 *      Author: ivp
 */

#ifndef SRC_WGS84POLYGON_H_
#define SRC_WGS84POLYGON_H_

#include  "WGS84Point.h"
#include <vector>
namespace tmx {
namespace utils {

class WGS84Polygon {
public:
	WGS84Polygon();
	virtual ~WGS84Polygon();

	int npoints;

//	WGS84Polygon(WGS84Point[] points);

	bool IsPointInsidePoly(WGS84Point pointToTest,WGS84Point A, WGS84Point B, WGS84Point C, WGS84Point D);

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
	bool IsPointInsidePoly(WGS84Point pointToTest,std::vector<WGS84Point> polyPoints);

private:
	// @SerializedName("WGS84Points")
//	WGS84Point[] mWGS84Points;



};

}} // namespace tmx::utils

#endif /* SRC_WGS84POLYGON_H_ */
