/*
 * SampleData.cpp
 *
 *  Created on: May 16, 2016
 *      Author: ivp
 */

#include "SampleData.h"

namespace DatabasePlugin {

// This class is used to demonstrate how code could be shared between
// a plugin (in the src folder) and the unit tests (in the test folder).
// See header file.

SampleData::SampleData() :
	Value(456)
{
}

SampleData::~SampleData()
{
}

} /* namespace DatabasePlugin */
