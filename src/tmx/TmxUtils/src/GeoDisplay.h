/*
 * GeoDisplay.h
 *
 *  Created on: Oct 6, 2016
 *      Author: ivp
 */

#ifndef GEODISPLAY_H_
#define GEODISPLAY_H_

#include <stdint.h>
#include <string.h>
#include "WGS84Point.h"

namespace tmx {
namespace utils {

/*
 * GeoDisplay sends display commands to a display application using datagrams
 */

class GeoDisplay
{
private:

	static bool _enabled;
	static void SendDisplayMessage(const char *destinationAddress, uint16_t destinationPort,
		uint16_t msgType, uint16_t msgVersion, uint16_t msgId, uint16_t msgDataLength, char* dataBuffer);

public:
	static void Enable();
	static void Disable();
	static void SendDisplayClearScreen(const char *destinationAddress, uint16_t destinationPort);
	static void SendDisplayPoint(const char *destinationAddress, uint16_t destinationPort,
			WGS84Point point, char pixelRadius, char color);
	static void SendDisplayLine(const char *destinationAddress, uint16_t destinationPort,
			WGS84Point point1, WGS84Point point2, char color);
	static void SendDisplayCircle(const char *destinationAddress, uint16_t destinationPort,
			WGS84Point point, uint32_t radiusInCentimeters, char color);
	static void SendDisplayString(const char *destinationAddress, uint16_t destinationPort,
			uint32_t screenX, uint32_t screenY, char color, char fontType, char fontSize, const char *displayString);
	static void SendRender(const char *destinationAddress, uint16_t destinationPort);

};

}} // namespace tmx::utils

#endif /* GEODISPLAY_H_ */
