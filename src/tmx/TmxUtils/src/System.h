/*
 * System.h
 *
 *  Created on: Sep 28, 2016
 *      Author: ivp
 */

#ifndef SRC_SYSTEM_H_
#define SRC_SYSTEM_H_

#include <string>

namespace tmx {
namespace utils {

class System
{
public:
	/**
	 * Execute a system command.
	 * @param command The command to execute.
	 * @return The output of the command.
	 */
	static std::string ExecCommand(const std::string& command, int *exitCode = NULL);

};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_SYSTEM_H_ */
