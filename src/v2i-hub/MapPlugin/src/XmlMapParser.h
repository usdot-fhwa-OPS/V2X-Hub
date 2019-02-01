#ifndef XML_MAP_PARSER_H
#define XML_MAP_PARSER_H

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMDocumentType.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/XMLUni.hpp>

#include <string>
#include <stdexcept>

#include "utils/common.h"
#include "utils/map.h"

namespace MapPlugin
{

// Error codes

enum {
	ERROR_ARGS = 1, ERROR_XERCES_INIT, ERROR_PARSE, ERROR_EMPTY_DOCUMENT
};

/**
 * \ingroup MAPPlugin
 * The XMLMapParser reads a xml representation of the Battelle SPaT MAP message from a file and
 * creates a map message.
 */
class XmlMapParser {
public:
	XmlMapParser();
	~XmlMapParser();

	bool ReadGidFile(std::string& filePath, map* message);

private:
	void ReadRoot(xercesc::DOMElement* root, map* message);
	void ReadGeometry(xercesc::DOMElement* geometryElement, map_geometry* geometry);
	void ReadReferencePoint(xercesc::DOMElement* referencePointElement, map_referencepoint* referencePoint);
	void ReadGroup(xercesc::DOMElement* groupElement, map_group* group);
	void ReadLane(xercesc::DOMElement* laneElement, map_lane* lane);
	void ReadReferenceLane(xercesc::DOMElement* referenceLaneElement, map_referencelane* referenceLane);
	void ReadNodes(xercesc::DOMElement* nodesElement, map_node* nodeArray);
	void ReadNode(xercesc::DOMElement* nodeElement, map_node* node);
	void ReadConnections(xercesc::DOMElement* connectionsElement, map_connection* connectionArray);
	void ReadConnection(xercesc::DOMElement* connectionElement, map_connection* connection);
	void ReadBarrier(xercesc::DOMElement* barrierElement, map_barrier* barrier);

	xercesc::XercesDOMParser *_parser;
};

} /* End namespace MapPlugin */

#endif
