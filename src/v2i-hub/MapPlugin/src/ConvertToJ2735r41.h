#ifndef CONVERT_TO_J2735_R41_H
#define CONVERT_TO_J2735_R41_H

#include <string>
#include <stdexcept>

#include "utils/common.h"
#include "utils/map.h"

#include "inputs/isd/ISDDataAdaptor.hpp"

namespace MapPlugin
{

class ConvertToJ2735r41 {
public:
	ConvertToJ2735r41();
	~ConvertToJ2735r41();

	int encodedByteCount;
	int derEncodedByteCount;
	unsigned char encoded[4000];

	int convertMap(map *mapMessage);
	bool decodePerMapData(MapData* map, char* messageBuf, int bytes);
private:
	void addLaneType(GenericLane *lane, BIT_STRING_t *choice, LaneTypeAttributes_PR laneType, uint16_t attributes);
	void addManeuvers(GenericLane *lane, uint16_t attributes);
	uint16_t	createVehicleLaneAttributes(unsigned short xmlAttributes);
	uint16_t	createCrosswalkLaneAttributes(unsigned short xmlAttributes);
	uint16_t	createAllowedManeuvers(unsigned short xmlAttributes);
	uint16_t	createTrackedVehicleAttributes(unsigned short xmlAttributes);

	void		printUINT16(uint16_t num);
	void convertIntersectionMap(MapData* MapJ2735, map* mapMessage);
	void convertRoadwayMap(MapData* MapJ2735, map* mapMessage);
	void LoadGenericLaneData(int laneIter, int isEgress, GenericLane* lane,
			map_group* geomMapGroup);
};

} /* End namespace MapPlugin */

#endif
