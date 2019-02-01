#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "XmlMapParser.h"

using namespace xercesc;
using namespace std;

namespace MapPlugin
{

XmlMapParser::XmlMapParser()
{
	try
	{
		// Initialize Xerces infrastructure.
		XMLPlatformUtils::Initialize();
	}
	catch (XMLException& e)
	{
		char* message = XMLString::transcode(e.getMessage());
		cerr << "XML toolkit initialization error: " << message << endl;
		XMLString::release(&message);
		// throw exception here to return ERROR_XERCES_INIT
	}

	_parser = new XercesDOMParser;
}

XmlMapParser::~XmlMapParser()
{
	// Free memory

	delete _parser;

	// Terminate Xerces framework.

	try
	{
		XMLPlatformUtils::Terminate();
	}
	catch (xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());

		cerr << "XML xerces termination error: " << message << endl;
		XMLString::release(&message);
	}
}

string GetFileError()
{
	// errno is declared by include file errno.h.
	if ( errno == ENOENT)
		return "File does not exist, or path is an empty string.";
	if ( errno == ENOTDIR)
		return "A component of the path is not a directory.";
	if ( errno == ELOOP)
		return "Too many symbolic links encountered while traversing the path.";
	if ( errno == EACCES)
		return "Permission denied.";
	if ( errno == ENAMETOOLONG)
		return "Filename is too long.";

	return "Unknown error reading file.";
}

bool XmlMapParser::ReadGidFile(string& filePath, map* message)
{
	// Test the file status.
	struct stat fileStatus;
	errno = 0;
	// ok == 0, error == -1.
	if (stat(filePath.c_str(), &fileStatus) == -1)
	{
		cout << GetFileError() << " File: " << filePath << endl;
		return false;
	}

	try
	{
		_parser->parse(filePath.c_str());

		// Get the DOM document.
		// It is owned by the parser, so there is no need to free it later.
		DOMDocument* xmlDoc = _parser->getDocument();

		// Get the top-level root element.
		DOMElement* root = xmlDoc->getDocumentElement();
		if (!root)
		{
			cout << "Root element not found.  Is XML document empty?" << " File: " << filePath << endl;
			return false;
		}

		ReadRoot(root, message);
	}
	catch (xercesc::XMLException& e)
	{
		char* message = xercesc::XMLString::transcode(e.getMessage());
		ostringstream errBuf;
		errBuf << "Error parsing file: " << message << flush;
		XMLString::release(&message);
		return false;
	}

	return true;
}

// Compare lower case versions of the elements tag name and a char* string.
bool MatchTagName(DOMElement* element, const char* name)
{
	XMLCh* xmlName = XMLString::transcode(name);
	bool isMatch = XMLString::compareIString(xmlName, element->getTagName()) == 0;
	XMLString::release(&xmlName);
	return isMatch;
}

// Compare lower case versions of the elements text content and a char* string.
bool MatchTextContent(DOMElement* element, const char* str)
{
	XMLCh* xmlStr = XMLString::transcode(str);
	bool isMatch = XMLString::compareIString(element->getTextContent(), xmlStr) == 0;
	XMLString::release(&xmlStr);
	return isMatch;
}

// Compare lower case versions of 2 strings (1 XMLCh* and 1 char*).
bool MatchString(const XMLCh* xmlCh, const char* str)
{
	XMLCh* xmlStr = XMLString::transcode(str);
	bool isMatch = XMLString::compareIString(xmlCh, xmlStr) == 0;
	XMLString::release(&xmlStr);
	return isMatch;
}

char* GetAttribute(DOMElement* element, const char* name)
{
	XMLCh* xmlName = XMLString::transcode(name);
	const XMLCh* attribute = element->getAttribute(xmlName);
	XMLString::release(&xmlName);
	return XMLString::transcode(attribute);
}

void XmlMapParser::ReadRoot(DOMElement* root, map* message)
{
	map_initialize(message);
	message->attributes = geometric | navigational;

	// Get all children of root.
	DOMNodeList* children = root->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	int geometryIndex = 0;

	// Loop through children of root.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		// This node is an Element.
		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "IntersectionID"))
		{
			message->intersectionid = atoi(XMLString::transcode(currentElement->getTextContent()));
		}
		else if (MatchTagName(currentElement, "MapId"))
		{
			message->mapid = atoi(XMLString::transcode(currentElement->getTextContent()));
		}
		else if (MatchTagName(currentElement, "MapType"))
		{
			if (MatchTextContent(currentElement, "Roadway"))
				message->mapType = roadway;
			else if (MatchTextContent(currentElement, "Intersection"))
				message->mapType = intersection;
			else
				message->mapType = intersection;
		}
		else if (MatchTagName(currentElement, "MapName"))
		{
			char* xmlName = XMLString::transcode(currentElement->getTextContent());
			strncpy ( (char*)message->mapName, (char*)xmlName, sizeof(message->mapName) );
			//XMLString::copyString((char*)message->mapName, (char*)xmlName);
			XMLString::release(&xmlName);

			printf("Map Name = %s\n", message->mapName);
		}
		else if (MatchTagName(currentElement, "Elevation"))
		{
			if (MatchTextContent(currentElement, "true"))
				message->attributes |= elevation;
		}
		else if (MatchTagName(currentElement, "Resolution"))
		{
			if (MatchTextContent(currentElement, "decimeter"))
				message->attributes |= decimeter;
		}
		else if (MatchTagName(currentElement, "Geometry"))
		{
			ReadGeometry(currentElement, &message->geometry[geometryIndex++]);
		}
	}
}

void XmlMapParser::ReadGeometry(DOMElement* geometryElement, map_geometry* geometry)
{
	// Get all children of Geometry.
	DOMNodeList* children = geometryElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	int barrierIndex = 0;

	// Loop through children of Geometry.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "ReferencePoint"))
		{
			ReadReferencePoint(currentElement, &geometry->refpoint);
		}
		else if (MatchTagName(currentElement, "Approach"))
		{
			ReadGroup(currentElement, &geometry->approach);
		}
		else if (MatchTagName(currentElement, "Egress"))
		{
			ReadGroup(currentElement, &geometry->egress);
		}
		else if (MatchTagName(currentElement, "Barrier"))
		{
			ReadBarrier(currentElement, &geometry->barrier[barrierIndex++]);
		}
	}
}

void XmlMapParser::ReadReferencePoint(DOMElement* referencePointElement, map_referencepoint* referencePoint)
{
	// Get all children of ReferencePoint.
	DOMNodeList* children = referencePointElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of ReferencePoint.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Latitude"))
		{
			referencePoint->latitude = (int)(atof(XMLString::transcode(currentElement->getTextContent())) * 10000000);
			//cout << "ReferencePoint Latitude: " << referencePoint->latitude << endl;
		}
		else if (MatchTagName(currentElement, "Longitude"))
		{
			referencePoint->longitude = (int)(atof(XMLString::transcode(currentElement->getTextContent())) * 10000000);
			//cout << "ReferencePoint Longitude" << referencePoint->longitude << endl;
		}
		else if (MatchTagName(currentElement, "Elevation"))
		{
			referencePoint->elevation = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "ReferencePoint Elevation: " << referencePoint->elevation << endl;
		}
	}
}

void XmlMapParser::ReadGroup(DOMElement* groupElement, map_group* group)
{
	// Get all children of Group.
	DOMNodeList* children = groupElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	int laneIndex = 0;

	// Loop through children of Group.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		// This node is an Element.
		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Width"))
		{
			group->width = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Group Width: " << group->width << endl;
		}
		else if (MatchTagName(currentElement, "Lane"))
		{
			ReadLane(currentElement, &group->lane[laneIndex++]);
		}
	}
}

void XmlMapParser::ReadLane(DOMElement* laneElement, map_lane* lane)
{
	lane->number = atoi(GetAttribute(laneElement, "Number"));
	cout << "Lane Number: " << (int)(lane->number) << endl;

	// Get all children of Lane.
	DOMNodeList* children = laneElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Lane.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Type"))
		{
			if (MatchTextContent(currentElement, "vehicle"))
				lane->type = vehicle;
			else if (MatchTextContent(currentElement, "computed"))
				lane->type = computed;
			else if (MatchTextContent(currentElement, "pedestrian"))
				lane->type = pedestrian;
			else if (MatchTextContent(currentElement, "special"))
				lane->type = special;
			else if (MatchTextContent(currentElement, "crosswalk"))
				lane->type = crosswalk;
			else if (MatchTextContent(currentElement, "bike"))
				lane->type = bike;
			else if (MatchTextContent(currentElement, "sidewalk"))
				lane->type = sidewalk;
			else if (MatchTextContent(currentElement, "barrier"))
				lane->type = barrier;
			else if (MatchTextContent(currentElement, "striping"))
				lane->type = striping;
			else if (MatchTextContent(currentElement, "trackedVehicle"))
				lane->type = trackedVehicle;
			else if (MatchTextContent(currentElement, "parking"))
				lane->type = parking;
		}
		else if (MatchTagName(currentElement, "LaneName"))
		{
			char* xmlName = XMLString::transcode(currentElement->getTextContent());
			strncpy ( (char*)lane->laneName, (char*)xmlName, sizeof(lane->laneName) );
			//XMLString::copyString((char*)lane->laneName, (char*)xmlName);
			XMLString::release(&xmlName);

			printf("Lane Name = %s\n", lane->laneName);
		}
		else if (MatchTagName(currentElement, "Attributes"))
		{
			lane->attributes = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Lane Attributes: " << lane->attributes << endl;
		}
		else if (MatchTagName(currentElement, "Width"))
		{
			lane->width = atoi(XMLString::transcode(currentElement->getTextContent()));
//			cout << "Lane Width: " << lane->width << endl;
		}
		else if (MatchTagName(currentElement, "ReferenceLane"))
		{
			ReadReferenceLane(currentElement, &lane->referencelane);
		}
		else if (MatchTagName(currentElement, "Nodes"))
		{
			ReadNodes(currentElement, lane->node);
		}
		else if (MatchTagName(currentElement, "Connections"))
		{
			ReadConnections(currentElement, lane->connection);
		}
	}
}

void XmlMapParser::ReadReferenceLane(DOMElement* referenceLaneElement, map_referencelane* referenceLane)
{
	// Get all children of ReferenceLane.
	DOMNodeList* children = referenceLaneElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of ReferenceLane.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "LaneNumber"))
		{
			referenceLane->lanenumber = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "ReferenceLane LaneNumber: " << (int)referenceLane->lanenumber << endl;
		}
		else if (MatchTagName(currentElement, "XOffset"))
		{
			referenceLane->xoffset = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "ReferenceLane LateralOffset: " << referenceLane->xoffset << endl;
		}
		else if (MatchTagName(currentElement, "YOffset"))
		{
			referenceLane->yoffset = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "ReferenceLane LateralOffset: " << referenceLane->yoffset << endl;
		}
	}
}

void XmlMapParser::ReadNodes(DOMElement* nodesElement, map_node* nodeArray)
{
	// Get all children of Nodes.
	DOMNodeList* children = nodesElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	int nodeIndex = 0;

	// Loop through children of Nodes.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Node"))
		{
			ReadNode(currentElement, &nodeArray[nodeIndex++]);
		}
	}
}

void XmlMapParser::ReadNode(DOMElement* nodeElement, map_node* node)
{
	// Get all children of Node.
	DOMNodeList* children = nodeElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Node.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Eastern"))
		{
			node->eastern = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Node Eastern: " << node->eastern << endl;
		}
		else if (MatchTagName(currentElement, "Northern") || MatchTagName(currentElement, "Nothern"))
		{
			// Misspelled Northern for backward compatibility.
			node->northern = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Node Northern: " << node->northern << endl;
		}
		else if (MatchTagName(currentElement, "Elevation"))
		{
			node->elevation = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Node Elevation: " << node->elevation << endl;
		}
		else if (MatchTagName(currentElement, "Width"))
		{
			node->width = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Node Width: " << node->width << endl;
		}
	}
}

void XmlMapParser::ReadConnections(DOMElement* connectionsElement, map_connection* connectionArray)
{
	// Get all children of Connections.
	DOMNodeList* children = connectionsElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	int connectionIndex = 0;

	// Loop through children of Connections.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Connection"))
		{
			ReadConnection(currentElement, &connectionArray[connectionIndex++]);
		}
	}
}

void XmlMapParser::ReadConnection(DOMElement* connectionElement, map_connection* connection)
{
	// Get all children of Connection.
	DOMNodeList* children = connectionElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Connection.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "LaneNumber"))
		{
			connection->lanenumber = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "Connection Lane Number: " << (int)connection->lanenumber << endl;
		}
		else if (MatchTagName(currentElement, "Maneuver"))
		{
			connection->maneuver = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "Connection Maneuver: " << (int)connection->maneuver << endl;
		}
		else if (MatchTagName(currentElement, "SignalGroup"))
		{
			connection->signalGroup = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "Connection Signal Group: " << (int)connection->signalGroup << endl;
		}
		else if (MatchTagName(currentElement, "ConnectionID"))
		{
			connection->connectionID = atoi(XMLString::transcode(currentElement->getTextContent()));
			cout << "Connection Signal ID: " << (int)connection->connectionID << endl;
		}
	}
}

void XmlMapParser::ReadBarrier(DOMElement* barrierElement, map_barrier* barrier)
{
	// Get all children of Barrier.
	DOMNodeList* children = barrierElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Barrier.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Attributes"))
		{
			barrier->attributes = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Barrier Attributes: " << barrier->attributes << endl;
		}
		else if (MatchTagName(currentElement, "Width"))
		{
			barrier->width = atoi(XMLString::transcode(currentElement->getTextContent()));
			//cout << "Barrier Width: " << barrier->width << endl;
		}
		else if (MatchTagName(currentElement, "Nodes"))
		{
			ReadNodes(currentElement, barrier->node);
		}
	}
}

} /* End namespace MapPlugin */
