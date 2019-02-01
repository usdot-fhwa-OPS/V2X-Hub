/*
 * StringUtils.cpp
 *
 *  Created on: Aug 1, 2014
 *      Author: ivp
 */

#include "StringUtils.h"

using namespace std;

std::vector<std::string> StringUtils::tokenize(std::string str, std::string sep, bool removeEmpty)
{
	vector<string> results;

	unsigned int startPosition = 0;

	unsigned int i;
	for(i = 0; i < str.length(); i++)
	{
		if (sep.find(str.at(i)) != string::npos)
		{
			results.push_back(str.substr(startPosition, i - startPosition));
			startPosition = i + 1;
		}
	}
	if (i != startPosition)
		results.push_back(str.substr(startPosition, i - startPosition));

	if (removeEmpty)
	{
		for(vector<string>::iterator itr = results.begin(); itr != results.end() && results.size() > 0; itr++)
		{
			if (itr->empty())
			{
				results.erase(itr);
				itr = results.begin();
			}
		}
	}

	return results;
}
