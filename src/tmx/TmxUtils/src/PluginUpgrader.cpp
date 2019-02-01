/*
 * @file PluginUpgrader.cpp
 *
 *  Created on: Aug 5, 2016
 *      @author: Gregory M. Baumgardner
 */

#include "PluginUpgrader.h"
#include "PluginLog.h"

#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <memory>

namespace tmx {
namespace utils {

#define ADD_GLOBAL_ENTRY \
	"INSERT INTO `IVP`.`plugin` \
	( `id`, `name`, `description`, `version` ) VALUES \
	( '0', 'Plugin System', 'The global configuration for all TMX plugins', ? ) \
	ON DUPLICATE KEY UPDATE name = VALUES(name), \
	description = VALUES(description), version = VALUES(version)"
#define UPDATE_GLOBAL_ENTRY \
	"UPDATE `IVP`.`plugin` \
	SET `id` = '0' \
	WHERE `name` = 'Plugin System'"
#define CHECK_MILLISEC_COLUMN \
	"SELECT count(*) FROM INFORMATION_SCHEMA.Columns \
	WHERE TABLE_SCHEMA = 'IVP' AND \
	TABLE_NAME = 'eventLog' AND \
	COLUMN_NAME = 'milliSeconds'"
#define ADD_MILLISEC_COLUMN \
	"ALTER TABLE `IVP`.`eventLog` \
	ADD COLUMN `milliSeconds` \
	INT(6) NOT NULL DEFAULT 0"

using namespace std;
using namespace sql;

void PluginUpgrader::UpgradeDatabase(DbConnection *conn, string newVersion) {
	if (conn && conn->IsConnected()) {
		// Version independent upgrades

		PLOG(logDEBUG2) << "Trying to upgrade the database.";

		unique_ptr<Statement> stmt;
		unique_ptr<PreparedStatement> pstmt;

		// 1. Add a new column to pluginConfigurationParameter
		pstmt.reset(conn->Get()->prepareStatement(ADD_GLOBAL_ENTRY));
		pstmt->setString(1, newVersion);
		pstmt->executeUpdate();

		stmt.reset(conn->Get()->createStatement());
		stmt->executeUpdate(UPDATE_GLOBAL_ENTRY);
/*
		// 2. Add a new microsecond column to the eventLog table
		rs.reset(stmt->executeQuery(CHECK_MILLISEC_COLUMN));
		doIt = true;
		if (rs && rs->next())
			doIt = rs->getInt(1) > 0;
		rs->close();

		if (doIt) {
			stmt->executeUpdate(ADD_MILLISEC_COLUMN);
		}
		*/
	}
}

} /* namespace utils */
} /* namespace tmx */
