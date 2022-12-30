/*
 * ImmediateForward.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: ivp
 */

#include "ImmediateForwardPlugin.h"

using namespace tmx::utils;
using namespace ImmediateForward;

// The main entry point for this application.
int main(int argc, char *argv[])
{
	return run_plugin<ImmediateForwardPlugin>("Immediate Forward", argc, argv);
}
