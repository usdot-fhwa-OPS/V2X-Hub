/*
 * SampleData.h
 *
 *  Created on: May 16, 2016
 *      Author: ivp
 */

#ifndef SAMPLEDATA_H_
#define SAMPLEDATA_H_

namespace ExamplePlugin {

/**
 * This is a class used purely to demonstrate how code could be shared between
 * a plugin (in the src folder) and the unit tests (in the test folder).
 *
 * While this class is not actually used by ExamplePlugin.cpp, it could be, since it
 * is located in the src folder and is linked into the main executable.
 * The unit test executable also includes this file, to demonstrate it can be included
 * by the unit test as well as the main executable.
 */
class SampleData {
public:
	SampleData();
	virtual ~SampleData();

	int Value;
};

} /* namespace ExamplePlugin */

#endif /* SAMPLEDATA_H_ */
