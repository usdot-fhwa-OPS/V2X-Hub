/*
 * TmxControlMain.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: Ben Paselsky
 */

#include "TmxControl.h"

using namespace std;
using namespace boost::program_options;
using namespace tmx;
using namespace tmx::utils;

int main(int argc, char *argv[])
{
	FILELog::ReportingLevel() = logERROR;
	Output2Eventlog::Enable() = true;

	try
	{
		tmxctl::TmxControl myExec;
		exit (run("", argc, argv, myExec));
	}
	catch (exception &ex)
	{
		cerr << ExceptionToString(ex) << endl;
		throw;
	}
}

