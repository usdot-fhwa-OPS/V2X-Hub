#ifndef XML_CURVE_PARSER_H
#define XML_CURVE_PARSER_H

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

#include "DsrcBuilder.h"

// Error codes

enum {
	ERROR_ARGS = 1, ERROR_XERCES_INIT, ERROR_PARSE, ERROR_EMPTY_DOCUMENT
};

class XmlCurveParser {
public:
	XmlCurveParser();
	XmlCurveParser(const XmlCurveParser& mn)
	{
		
	}


	~XmlCurveParser();

	bool ReadCurveFile(const char *filePath, TravelerInformation *tim);

	unsigned int SpeedLimit;

private:
	void ReadRoot(xercesc::DOMElement* root, TravelerInformation *tim);
	void ReadApproaches(xercesc::DOMElement* approachesElement, TravelerInformation *tim);
	void ReadApproach(xercesc::DOMElement* approachElement, TravelerInformation *tim);
	TiDataFrame* ReadRegions(xercesc::DOMElement* regionsElement);
	GeographicalPath* ReadRegion(xercesc::DOMElement* geometryElement);
	Position3D* ReadReferencePoint(xercesc::DOMElement* referencePointElement);
	void ReadNodes(xercesc::DOMElement* nodesElement, NodeSetXY *nodeSet);
	void ReadNode(xercesc::DOMElement* nodeElement, NodeSetXY *nodeSet);

	xercesc::XercesDOMParser *_parser;
};

#endif
