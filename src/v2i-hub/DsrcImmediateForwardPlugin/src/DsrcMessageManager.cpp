/*
 * DsrcMessageManager.cpp
 *
 *  Created on: Feb 26, 2016
 *      Author: ivp
 */

#include "DsrcMessageManagerPlugin.h"

using namespace tmx::utils;
using namespace DsrcMessageManager;

// The main entry point for this application.
int main(int argc, char *argv[])
{
	return run_plugin<DsrcMessageManagerPlugin>("DSRC Message Manager", argc, argv);
}
