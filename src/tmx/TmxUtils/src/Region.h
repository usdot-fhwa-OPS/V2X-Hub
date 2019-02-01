/*
 * Region.h
 *
 *  Created on: Apr 29, 2016
 *      Author: ivp
 */

#ifndef SRC_REGION_H_
#define SRC_REGION_H_
	#include <list>

namespace tmx {
namespace utils {

/**
 * Regions make up Intersections, a standard intersection has four regions.
 *   They keep track of all the different types of lane Ids that comprise the region.
 */
class Region
{
public:
	Region(int regionId) : RegionId(regionId) {};
	~Region() {};

	///Contains a list of all lane Ids from the MAP message that correspond to crosswalks.
	std::list<int> Crosswalks;
	///Contains a list of all lane Ids from the MAP message that correspond to curbs/sidewalks.
	std::list<int> Curbs;
	///Contains a list of all lane Ids from the MAP message that correspond to roadway/vehicle lanes.
	std::list<int> VehicleLanes;

	///Numeric region Id corresponding to the region of the intersection. Unique per intersection.
	const int RegionId = -1;
private:

};

}} // namespace tmx::utils

#endif /* SRC_REGION_H_ */
