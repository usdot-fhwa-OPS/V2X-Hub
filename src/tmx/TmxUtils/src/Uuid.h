/*
 * Uuid.h
 *
 *  Created on: Aug 19, 2016
 *      Author: ivp
 */

#ifndef SRC_UUID_H_
#define SRC_UUID_H_

#include <string>
#include <uuid/uuid.h>

namespace tmx {
namespace utils {

class Uuid {
public:
	static std::string NewGuid()
	{
		char str[37];
		uuid_t id;
		uuid_generate(id);
		uuid_unparse(id,str);
	    return std::string(str);
	}
};

}} // namespace tmx::utils

#endif /* SRC_UUID_H_ */
