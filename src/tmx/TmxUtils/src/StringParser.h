/*
 * StringParser.h
 *
 *  Created on: Aug 24, 2016
 *      Author: ivp
 */

#ifndef SRC_STRINGPARSER_H_
#define SRC_STRINGPARSER_H_

#include <string>

namespace tmx {
namespace utils {

class StringParser {
public:
	/**
	 * Get a substring from within a string that is delimited by a start and end token.
	 *
	 * @param str The string to search.
	 * @param startToken The string that immediately precedes the substring to extract.
	 * @param endToken The String that immediately follows the substring to extract.
	 * @param isEndTokenOptional When true, if the endToken is not found, then the remainder of the string is returned.
	 * @return The substring between the start and end token or from start token to end of string or an empty string if not found.
	 */
	static std::string Substring(std::string str, std::string startToken, std::string endToken, bool isEndTokenOptional = true);
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_STRINGPARSER_H_ */
