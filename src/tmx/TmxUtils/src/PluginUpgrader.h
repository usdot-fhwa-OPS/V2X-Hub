/*
 * @file PluginUpgrader.h
 *
 *  Created on: Aug 5, 2016
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_PLUGINUPGRADER_H_
#define SRC_PLUGINUPGRADER_H_

#include "database/DbConnection.h"

namespace tmx {
namespace utils {

class PluginUpgrader {
public:
	static void UpgradeDatabase(DbConnection *, std::string);
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_PLUGINUPGRADER_H_ */
