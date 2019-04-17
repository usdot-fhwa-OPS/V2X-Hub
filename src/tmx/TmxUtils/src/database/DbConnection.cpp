/*
 * @file DatabaseConnection.cpp
 *
 *  Created on: Aug 5, 2016
 *      @author: Gregory M. Baumgardner
 */

#include "DbConnection.h"

#include <cppconn/statement.h>
#include <mutex>
#include <mysql_driver.h>
#include "../PluginLog.h"

#define DEFAULT_USER "IVP"
#define DEFAULT_PWD ""
#define DEFAULT_DB "IVP"

namespace tmx {
namespace utils {

using namespace std;
using namespace sql;

std::mutex dataLock;

DbConnection::DbConnection() {
}

DbConnection::~DbConnection() {
	//if (_connection && !_connection->isClosed())
	//	_connection->close();
}

DbConnection::DbConnection(string connectionUrl, string username, string password, string db) {
	Init(connectionUrl, username, password, db);
}

DbConnection::DbConnection(const DbConnection &copy): _connection(copy._connection) {
	Init(copy._connInfo.url, copy._connInfo.username, copy._connInfo.password, copy._connInfo.db);
}

void DbConnection::Init(string connectionUrl, string username, string password, string db) {
	_connInfo.url = connectionUrl;
	_connInfo.username = (username.empty() ? DEFAULT_USER : username);
	_connInfo.password = (password.empty() ? DEFAULT_PWD : password);
	_connInfo.db = db;

	Reconnect();
}

Connection *DbConnection::Get() {
	return _connection.get();
}

bool DbConnection::IsConnected() {
	return (_connection && !(_connection->isClosed()));
}

bool DbConnection::Reconnect() {
	lock_guard<mutex> lock(dataLock);

	// If the connection is already active and nothing has changed, do nothing
	if (IsConnected() &&
			_oldInfo.url == _connInfo.url &&
			_oldInfo.username == _connInfo.username &&
			_oldInfo.password == _connInfo.password) {
		return false;
	}

	PLOG(logDEBUG) << "Attempting to connect to " <<
			_connInfo.url <<  " " << _connInfo.db;

	// Otherwise, reconnect
	_connection.reset(mysql::get_mysql_driver_instance()->connect(
			_connInfo.url, _connInfo.username, _connInfo.password));

	// Set the database
	SetDatabase(_connInfo.db);

	_oldInfo.url = _connInfo.url;
	_oldInfo.username = _connInfo.username;
	_oldInfo.password = _connInfo.password;

	return true;
}

std::string DbConnection::GetConnectionUrl() {
	return _connInfo.url;
}

void DbConnection::SetConnectionUrl(string connectionUrl) {
	_connInfo.url = connectionUrl;

	if (IsConnected())
		Reconnect();
}

std::string DbConnection::GetDatabase() {
	return _connInfo.db;
}

void DbConnection::SetDatabase(string db) {
	_connInfo.db = db;

	if (IsConnected() && !_connInfo.db.empty() && _connInfo.db != _oldInfo.db) {
		// Set the database on this connection
		std::unique_ptr< sql::Statement > stmt(_connection->createStatement());
		stmt->execute("USE " + _connInfo.db);
	}

	_oldInfo.db = _connInfo.db;
}

} /* namespace utils */
} /* namespace tmx */
