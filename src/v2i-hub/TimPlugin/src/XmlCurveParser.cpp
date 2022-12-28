#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <list>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <NodeOffsetPointXY.h>
#include <GeographicalPath.h>
#include <OffsetSystem.h>
#include <NodeListXY.h>

#include "XmlCurveParser.h"

#include <OCTET_STRING.h>

using namespace xercesc;
using namespace std;

bool _debugOutput = true;

XmlCurveParser::XmlCurveParser() :
		SpeedLimit(0)
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

XmlCurveParser::~XmlCurveParser()
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

bool XmlCurveParser::ReadCurveFile(const char *filePath, TravelerInformation *tim)
{
	cout<<"ReadCurveFile"<<endl;
	// Test the file status.
	struct stat fileStatus;
	errno = 0;
	// ok == 0, error == -1.
	if (stat(filePath, &fileStatus) == -1)
	{
		cout << GetFileError() << " File: " << filePath << endl;
		return false;
	}

	try
	{

		_parser->parse(filePath);


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

		ReadRoot(root, tim);
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

void XmlCurveParser::ReadRoot(DOMElement* root, TravelerInformation *tim)
{
	cout<<"GetChildNodes"<<endl;
	// Get all children of root.
	DOMNodeList* children = root->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	tim->timeStamp=NULL;
	tim->packetID=NULL;
	tim->urlB=NULL;
	tim->regional = NULL;

	unsigned int speedLimit = 0;

	// Loop through children of root.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		// This node is an Element.
		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "CurveID"))
		{
			unsigned long curveId = atoi(XMLString::transcode(currentElement->getTextContent()));
			if (_debugOutput)
				cout << "CurveID: " << curveId << endl;
		}
		else if (MatchTagName(currentElement, "SpeedLimit"))
		{
			speedLimit = atoi(XMLString::transcode(currentElement->getTextContent()));
			if (_debugOutput)
				cout << "SpeedLimit: " << speedLimit << endl;
		}
		else if (MatchTagName(currentElement, "Approaches"))
		{
			// Each approach read will add one data frame to the TIM.
			// An XML file should have either 1 or 2 approaches.
			ReadApproaches(currentElement, tim);
		}
	}

	SpeedLimit = speedLimit;

#if SAEJ2735_SPEC < 63
	tim->dataFrameCount = (Count_t *)calloc(1, sizeof(Count_t));
	*tim->dataFrameCount = tim->dataFrames.list.count;
#endif

	// Add Frame Part III (content) for each frame.
	for (int i = 0; i < tim->dataFrames.list.count; i++)
	{
		TiDataFrame *frame = tim->dataFrames.list.array[i];
		DsrcBuilder::AddTimAdvisory(frame, speedLimit);
	}
}

// Read Approaches from the XML file, store each approach in a data frame within the TIM.
void XmlCurveParser::ReadApproaches(DOMElement* approachesElement, TravelerInformation *tim)
{
	// Get all children of Approaches.
	DOMNodeList* children = approachesElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Approaches.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Approach"))
		{
			ReadApproach(currentElement, tim);
		}
	}
}

void XmlCurveParser::ReadApproach(DOMElement* approachElement, TravelerInformation *tim)
{
	// Get all children of Approaches.
	DOMNodeList* children = approachElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Approaches.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Regions"))
		{
			TiDataFrame *frame = ReadRegions(currentElement);
			asn_set_add(&tim->dataFrames.list, frame);
		}
	}
}

// Read Regions from the XML file, store them in a single frame, and return the frame.
TiDataFrame* XmlCurveParser::ReadRegions(DOMElement* regionsElement)
{
	TiDataFrame *frame = (TiDataFrame*)calloc(1, sizeof(TiDataFrame));

	frame->frameType = TravelerInfoType_advisory;

#if SAEJ2735_SPEC < 63
	frame->msgId.present = msgId_PR_furtherInfoID;
#else
	frame->msgId.present = TravelerDataFrame__msgId_PR_furtherInfoID;
	std::string tempString = "00";
	 OCTET_STRING_fromString(&(frame->msgId.choice.furtherInfoID), tempString.c_str());
#endif

	// TODO: Does this need set?
	//frame->msgId.choice.furtherInfoID =

	DsrcBuilder::SetStartTimeToYesterday(frame);

	// Set the duration (minutes) to its max value.
	frame->duratonTime = 32000;

	frame->startYear = NULL;
	frame->url = NULL;
	// Priority is 0-7 with 7 the highest priority.
	// TODO: How high should this be set?
	frame->priority = 5;

	// Get all children of Regions.
	DOMNodeList* children = regionsElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Regions.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Region"))
		{
			GeographicalPath *region = ReadRegion(currentElement);
			asn_set_add(&frame->regions.list, region);
		}
	}

	return frame;
}

GeographicalPath* XmlCurveParser::ReadRegion(DOMElement* regionElement)
{
	GeographicalPath *geoPath = (GeographicalPath*)malloc(sizeof(GeographicalPath));



	geoPath->description = (GeographicalPath::GeographicalPath__description*)
		malloc(sizeof(GeographicalPath::GeographicalPath__description));
	geoPath->regional = NULL;

	geoPath->name = NULL;
	geoPath->id = NULL;
	geoPath->directionality = NULL;
	geoPath->closedPath = NULL;
	geoPath->direction = NULL;

	memset(geoPath, 0, sizeof(geoPath));

	geoPath->description->present = GeographicalPath__description_PR_path;

	geoPath->description->choice.path.scale = NULL;
	geoPath->description->choice.path.offset.present = OffsetSystem__offset_PR_xy;
	geoPath->description->choice.path.offset.choice.xy.present = NodeListXY_PR_nodes;
	// Get all children of Region.
	DOMNodeList* children = regionElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Region.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "ReferencePoint"))
		{
			geoPath->anchor = ReadReferencePoint(currentElement);
		}
		else if (MatchTagName(currentElement, "Width"))
		{
			LaneWidth_t * laneWidth = (LaneWidth_t *)malloc(sizeof(LaneWidth_t));
			long lWidth = atol(XMLString::transcode(currentElement->getTextContent()));
			*laneWidth = lWidth;
			//if (_debugOutput)

			geoPath->laneWidth = laneWidth;
		}
		else if (MatchTagName(currentElement, "DirectionOfUse"))
		{
			DirectionOfUse_t* directionOfUse = (DirectionOfUse_t *)malloc(sizeof(DirectionOfUse_t));

			if (MatchTextContent(currentElement, "forward"))
			{
				cout<<"Forward"<<endl;
				*directionOfUse = DirectionOfUse_forward;
			}
			else if (MatchTextContent(currentElement, "reverse"))
			{
				cout<<"Reverse"<<endl;
				*directionOfUse = DirectionOfUse_reverse;
			}
			else if (MatchTextContent(currentElement, "both"))
				*directionOfUse = DirectionOfUse_both;
			else
			{
				cout<<"Unavailable"<<endl;
				*directionOfUse = DirectionOfUse_unavailable;
			}

			geoPath->directionality = directionOfUse;
		}
		else if (MatchTagName(currentElement, "Nodes"))
		{
			NodeSetXY_t* nodeset_p = (NodeSetXY_t*) calloc(1, sizeof(NodeSetXY));  
			ReadNodes(currentElement, nodeset_p);
			geoPath->description->choice.path.offset.choice.xy.choice.nodes = *nodeset_p;
			free(nodeset_p);
		}
	}

	return geoPath;
}

Position3D* XmlCurveParser::ReadReferencePoint(DOMElement* referencePointElement)
{
	Position3D *anchor = (Position3D*)malloc(sizeof(Position3D));

	anchor->regional = NULL;
	anchor->elevation = NULL;
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
			anchor->lat = (long)(atof(XMLString::transcode(currentElement->getTextContent())) * 10000000.0);
			if (_debugOutput)
				cout << "ReferencePoint Latitude: " << anchor->lat << endl;
		}
		else if (MatchTagName(currentElement, "Longitude"))
		{
			anchor->Long = (long)(atof(XMLString::transcode(currentElement->getTextContent())) * 10000000.0);
			if (_debugOutput)
				cout << "ReferencePoint Longitude" << anchor->Long << endl;
		}
		else if (MatchTagName(currentElement, "Elevation"))
		{
			int16_t elevation = atoi(XMLString::transcode(currentElement->getTextContent())) * 10;

#if SAEJ2735_SPEC < 63
			anchor->elevation = (Elevation_t *)malloc(sizeof(Elevation_t));
			anchor->elevation->buf = (uint8_t *)calloc(1,2);
			anchor->elevation->size = 2;

			anchor->elevation->buf[0] = elevation >> 8;
			anchor->elevation->buf[1] = elevation & 0xFF;
#else
			anchor->elevation = (DSRC_Elevation_t *)malloc(sizeof(DSRC_Elevation_t));
			*(anchor->elevation) = elevation;
#endif
			if (_debugOutput)
				cout << "ReferencePoint Elevation: " << elevation << endl;
		}
	}

	return anchor;
}

void XmlCurveParser::ReadNodes(DOMElement* nodesElement, NodeSetXY *nodeSet)
{
	// Get all children of Nodes.
	DOMNodeList* children = nodesElement->getChildNodes();
	const XMLSize_t childCount = children->getLength();

	// Loop through children of Nodes.
	for (XMLSize_t i = 0; i < childCount; i++)
	{
		DOMNode* currentNode = children->item(i);
		if (currentNode->getNodeType() != DOMNode::ELEMENT_NODE)
			continue;

		DOMElement* currentElement = dynamic_cast<xercesc::DOMElement*>(currentNode);

		if (MatchTagName(currentElement, "Node"))
		{
			ReadNode(currentElement, nodeSet);
		}
	}
}

void XmlCurveParser::ReadNode(DOMElement* nodeElement, NodeSetXY *nodeSet)
{
	NodeXY * node = (NodeXY *)malloc(sizeof(NodeXY));
	node->attributes = NULL;
	node->delta.present = NodeOffsetPointXY_PR_node_XY6;
	
	int16_t xOffset = 0;
	int16_t yOffset = 0;
	int16_t eOffset = -0;
	bool hasElevation = false;

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
			xOffset = atoi(XMLString::transcode(currentElement->getTextContent()));
			if (_debugOutput)
				cout << "Node Eastern: " << xOffset << endl;
			

		}
		else if (MatchTagName(currentElement, "Northern"))
		{
			yOffset = atoi(XMLString::transcode(currentElement->getTextContent()));
			if (_debugOutput)
				cout << "Node Northern: " << yOffset << endl;
		}
		else if (MatchTagName(currentElement, "Elevation"))
		{
			hasElevation = true;
			eOffset = atoi(XMLString::transcode(currentElement->getTextContent()));
			if (_debugOutput)
				cout << "Node Elevation: " << eOffset << endl;
		}
	}

	node->delta.choice.node_XY6.x = xOffset;
	node->delta.choice.node_XY6.y = yOffset;

	ASN_SEQUENCE_ADD(&nodeSet->list, node);
	

}
