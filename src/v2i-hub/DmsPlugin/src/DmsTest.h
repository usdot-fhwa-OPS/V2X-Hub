/*
 * DmsTest.h
 *
 *  Created on: Oct 12, 2015
 *      Author: ivp
 */

#ifndef DMSTEST_H_
#define DMSTEST_H_

#include "dmsNTCIP.h"

class DmsTest
{
public:
	DmsTest();
	virtual ~DmsTest();

	static void Test(SignalControllerNTCIP sc);
};

#endif /* DMSTEST_H_ */
