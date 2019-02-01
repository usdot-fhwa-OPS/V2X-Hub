/*
 * StringParser.cpp
 *
 *  Created on: Aug 24, 2016
 *      Author: ivp
 */

#include "StringParser.h"

using namespace std;

namespace tmx {
namespace utils {

std::string StringParser::Substring(std::string str, std::string startToken, std::string endToken, bool isEndTokenOptional)
{
	string result = "";

	size_t pos1 = str.find(startToken);
	if (pos1 == string::npos)
		return result;

	pos1 += startToken.length();
	size_t pos2 = str.find(endToken, pos1);

	if (pos2 != string::npos)
		result = str.substr(pos1, pos2-pos1);
	else if (isEndTokenOptional)
		result = str.substr(pos1);

	return result;
}

} /* namespace utils */
} /* namespace tmx */
