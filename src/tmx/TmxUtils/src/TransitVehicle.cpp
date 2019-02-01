/*
 * TransitVehicle.cpp
 *
 *  Created on: Apr 17, 2016
 *      Author: Svetlana Jacoby
 */
#include <string>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "PluginLog.h"
#include "TransitVehicle.h"


using namespace std;
using namespace boost;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::messages::appmessage;

namespace tmx {
namespace utils {

TransitVehicle::TransitVehicle(TransitVehicleState bState, uint64_t tStamp, string bName)
	: _state{bState}, _timestamp{tStamp}, _rawName{bName}
{
	construct();
}

TransitVehicle::TransitVehicle(EventCodeTypes ev, uint64_t tStamp, string bName)
	: _state{eventToState(ev)}, _timestamp{tStamp}, _rawName{bName}
{
	construct();
}

TransitVehicle::~TransitVehicle()
{}

void TransitVehicle::construct()
{
	/// Any of these formats are valid for rawName. ';' separated 'ID', 'Route' and 'Bus' in any order of occurance.
	/// "ID 3;Route 22;Bus 10092"
	/// "Route 22;Bus 10092"
	/// "Route 22;ID 3;Bus 10092"
	/// "Route 22;Bus 10092;ID 3"
	std::vector<std::string> fields;
	boost::split(fields, _rawName, is_any_of(";"));

	for(string field : fields)
	{
		FILE_LOG(logDEBUG2) << "Constructing from field: " << field <<endl;
		if (field.find("ID") != string::npos)
		{
			_id = stoi(field.substr(field.find(" ")+1).c_str());
			FILE_LOG(logDEBUG2) << "_id is set to: " << _id <<endl;
		}
		else if (field.find("Route") != string::npos)
		{
			_route = field.substr(field.find(" ")+1).c_str();
			FILE_LOG(logDEBUG2) << "_route is set to: " << _route <<endl;
		}
		else if (field.find("Bus") != string::npos)
		{
			_number = field.substr(field.find(" ")+1).c_str();
			FILE_LOG(logDEBUG2) << "_number is set to: " << _number <<endl;
		}
	}
}

}} // namespace tmx::utils

