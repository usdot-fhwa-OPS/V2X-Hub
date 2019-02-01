/*
 * StringUtils.h
 *
 *  Created on: Aug 1, 2014
 *      Author: ivp
 */

#ifndef STRINGUTILS_H_
#define STRINGUTILS_H_

#include <vector>
#include <string>

class StringUtils {
public:
	static std::vector<std::string> tokenize(std::string str, std::string sep, bool removeEmpty = false);
};

#endif /* STRINGUTILS_H_ */
