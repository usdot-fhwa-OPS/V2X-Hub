/*
 * GeoDisplay.cpp
 *
 *  Created on: Oct 6, 2016
 *      Author: ivp
 */

// Officially, the ANSI standard does not include the math constant definitions, such as M_PI used below
#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#include <cmath>
#define __STRICT_ANSI__
#else
#include <cmath>
#endif

#include "GeoDisplay.h"
#include "UdpClient.h"

using namespace std;

namespace tmx {
namespace utils {

/*
 * initialization of statics
 */
bool GeoDisplay::_enabled = false;

/*
 * format and send message to display application
 */
void GeoDisplay::SendDisplayMessage(const char *destinationAddress, uint16_t destinationPort,
		uint16_t msgType, uint16_t msgVersion, uint16_t msgId, uint16_t msgDataLength, char* dataBuffer)
{
	char* buffer;
	int bufferLength = 8 + msgDataLength;
	if (!_enabled)
		return;
	UdpClient client(string(destinationAddress), destinationPort);
	uint16_t networkShort;
	buffer = (char*)malloc(bufferLength);
	networkShort = htons(msgType);
	memcpy(&(buffer[0]), &networkShort, 2);
	networkShort = htons(msgVersion);
	memcpy(&(buffer[2]), &networkShort, 2);
	networkShort = htons(msgId);
	memcpy(&(buffer[4]), &networkShort, 2);
	networkShort = htons(msgDataLength);
	memcpy(&(buffer[6]), &networkShort, 2);
	if (msgDataLength > 0)
	{
		memcpy(&(buffer[8]), dataBuffer, msgDataLength);
	}
	client.Send(buffer, bufferLength);
	free(buffer);
}

/*
 * Enable sending of messages
 */
void GeoDisplay::Enable()
{
	_enabled = true;
}

/*
 * Disable sending of messages
 */
void GeoDisplay::Disable()
{
	_enabled = false;
}

/*
 * Clear display screen
 */
void GeoDisplay::SendDisplayClearScreen(const char *destinationAddress, uint16_t destinationPort)
{
	if (!_enabled)
		return;
	SendDisplayMessage(destinationAddress, destinationPort, 1, 1, 1, 0, 0);
}

/*
 * Draw a point
 */
void GeoDisplay::SendDisplayPoint(const char *destinationAddress, uint16_t destinationPort,
		WGS84Point point, char pixelRadius, char color)
{
	char* buffer;
	uint32_t networkLong;
	int32_t coord;
	if (!_enabled)
		return;
	buffer = (char*)malloc(10);
	coord = point.Longitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[0]), &networkLong, 4);
	coord = point.Latitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[4]), &networkLong, 4);
	memcpy(&(buffer[8]), &pixelRadius, 1);
	memcpy(&(buffer[9]), &color, 1);
	SendDisplayMessage(destinationAddress, destinationPort, 2, 1, 1, 10, buffer);
	free(buffer);
}

/*
 * Draw a line
 */
void GeoDisplay::SendDisplayLine(const char *destinationAddress, uint16_t destinationPort,
		WGS84Point point1, WGS84Point point2, char color)
{
	char* buffer;
	uint32_t networkLong;
	int32_t coord;
	if (!_enabled)
		return;
	buffer = (char*)malloc(17);
	coord = point1.Longitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[0]), &networkLong, 4);
	coord = point1.Latitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[4]), &networkLong, 4);
	coord = point2.Longitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[8]), &networkLong, 4);
	coord = point2.Latitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[12]), &networkLong, 4);
	memcpy(&(buffer[16]), &color, 1);
	SendDisplayMessage(destinationAddress, destinationPort, 3, 1, 1, 17, buffer);
	free(buffer);
}

/*
 * Draw a circle
 */
void GeoDisplay::SendDisplayCircle(const char *destinationAddress, uint16_t destinationPort,
		WGS84Point point, uint32_t radiusInCentimeters, char color)
{
	char* buffer;
	uint32_t networkLong;
	int32_t coord;
	if (!_enabled)
		return;
	buffer = (char*)malloc(13);
	coord = point.Longitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[0]), &networkLong, 4);
	coord = point.Latitude * 10000000;
	networkLong = htonl(coord);
	memcpy(&(buffer[4]), &networkLong, 4);
	networkLong = htonl(radiusInCentimeters);
	memcpy(&(buffer[8]), &networkLong, 4);
	memcpy(&(buffer[12]), &color, 1);
	SendDisplayMessage(destinationAddress, destinationPort, 4, 1, 1, 13, buffer);
	free(buffer);
}

/*
 * Draw a string
 */
void GeoDisplay::SendDisplayString(const char *destinationAddress, uint16_t destinationPort,
		uint32_t screenX, uint32_t screenY, char color, char fontType, char fontSize, const char *displayString)
{
	char* buffer;
	uint32_t networkLong;
	if (!_enabled)
		return;
	buffer = (char*)malloc(11 + strlen(displayString));
	networkLong = htonl(screenX);
	memcpy(&(buffer[0]), &networkLong, 4);
	networkLong = htonl(screenY);
	memcpy(&(buffer[4]), &networkLong, 4);
	memcpy(&(buffer[8]), &color, 1);
	memcpy(&(buffer[9]), &fontType, 1);
	memcpy(&(buffer[10]), &fontSize, 1);
	memcpy(&(buffer[11]), displayString, strlen(displayString));
	SendDisplayMessage(destinationAddress, destinationPort, 5, 1, 1, 11 + strlen(displayString), buffer);
	free(buffer);
}

/*
 * Render graphics (display is double buffered)
 */
void GeoDisplay::SendRender(const char *destinationAddress, uint16_t destinationPort)
{
	if (!_enabled)
		return;
	SendDisplayMessage(destinationAddress, destinationPort, 6, 1, 1, 0, 0);
}


}} // namespace tmx::utils
