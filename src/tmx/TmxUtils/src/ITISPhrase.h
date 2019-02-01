/*
 * ITISPhrase.h
 *
 *  Created on: Dec 6, 2017
 *      Author: ivp
 */

#ifndef SRC_ITISPHRASE_H_
#define SRC_ITISPHRASE_H_

#include <map>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace tmx {
namespace utils {



class ITISPhrase
{
private:

	static map<int, string> _codes;

public:

	static string GetPhrase(int code)
	{
		if (_codes.find(code) != _codes.end())
			return _codes[code];
		else
			return "";
	}

	static string GetPhrase(string code)
	{
		int i;
		istringstream iss(code);
		iss >> i;
		if (!iss.fail())
		{
			if (_codes.find(i) != _codes.end())
				return _codes[i];
			else
				return "";
		}
		else
			return "";
	}
};


}} // namespace tmx::utils

#endif /* SRC_ITISPHRASE_H_ */
