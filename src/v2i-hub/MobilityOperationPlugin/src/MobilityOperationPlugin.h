#ifndef SRC_MOBILITYOPERATIONPLUGIN_H_
#define SRC_MOBILITYOPERATIONPLUGIN_H_
#include "PluginClient.h"
#include <tmx/j2735_messages/testMessage03.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>



using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace boost::property_tree;


namespace MobilityOperationPlugin {
static CONSTEXPR const char *PORT_DRAYAGE_STRATEGY = "carma/port_drayage";

class MobilityOperationPlugin: public PluginClient {
public:
	struct PortDrayage_Object {
		int cmv_id; 
		int cargo_id;
		bool cargo;
		std::string operation;
		float location_lat;
		float location_long;
		float destination_lat;
		float destination_long;
		int action_id;
		int next_action;
	};

	MobilityOperationPlugin(std::string);
	virtual ~MobilityOperationPlugin();
	int Main();
protected:

	void UpdateConfigSettings();

	// Virtual method overrides.
	void OnConfigChanged(const char *key, const char *value);

	void OnStateChange(IvpPluginState state);

	void createPortDrayageJson( PortDrayage_Object &pd_obj, ptree &json_payload);

	void createMobilityOperationXml( ptree &mobilityOperationXml, ptree json_payload);

	void HandleMobilityOperationMessage(tsm3Message &msg, routeable_message &routeableMsg);

private:
	std::string _database_username;
	std::string _database_password;
	uint16_t _database_port;
	std::string _database_ip;
	std::string _database_name; 
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::PreparedStatement *pstmt;
	sql::ResultSet *res;
	J2735MessageFactory factory;

};
std::mutex _cfgLock;

}
#endif