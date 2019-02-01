/*
 * @file DatabaseConnection.h
 *
 *  Created on: Aug 5, 2016
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_DATABASE_DBCONNECTION_H_
#define SRC_DATABASE_DBCONNECTION_H_

#include <cppconn/connection.h>
#include <memory>

namespace tmx {
namespace utils {

struct DbConnectionInformation {
	std::string url;
	std::string username;
	std::string password;
	std::string db;
};

class DbConnectionPool;

/**
 * This class holds a connection to the TMX database.  The connection is not managed.
 */
class DbConnection {
	friend class DbConnectionPool;

public:
	/**
	 * Construct a new connection from the URL.  The connection is active upon construction.
	 *
	 * @param connectionUrl The URL to connect to
	 * @param username The user to login with, or the default
	 * @param password The password to login with, or the default
	 * @param db The active database to use, or none
	 * @throws A sql::SQLException for connection errors
	 */
	DbConnection(std::string connectionUrl, std::string username = "", std::string password = "", std::string db = "");

	/**
	 * Construct a duplicate connection from an active one.  The connections are shared.
	 *
	 * @param copy The existing connection
	 * @throws A sql::SQLException for connection errors
	 */
	DbConnection(const DbConnection &copy);

	virtual ~DbConnection();

	/**
	 * @return The under lying sql::Connection object, or NULL.
	 */
	sql::Connection *Get();

	/**
	 * @return De-reference the under lying sql::Connection object
	 */
	inline sql::Connection &operator*() {
		return *(Get());
	}

	/**
	 * @return The underlying sql::Connection object, or NULL.
	 */
	inline sql::Connection *operator->() {
		return Get();
	}

	/**
	 * @return True if the connection is active.
	 * @throws A sql::SQLException for connection errors
	 */
	bool IsConnected();

	/**
	 * Test the connection and reconnect if necessary.
	 *
	 * @return True if the connection was inactive and had to be reconnected.
	 * @throws A sql::SQLException for connection errors
	 *
	 */
	bool Reconnect();

	/**
	 * Retrieve the current connection URL.
	 *
	 * @return The URL associated with this connection
	 */
	std::string GetConnectionUrl();

	/**
	 * Change the connection URL.  The connection will automatically attempt to
	 * reconnect with the updated URL.
	 *
	 * @param The new URL to connect to
	 * @throws A sql::SQLException for connection errors
	 */
	void SetConnectionUrl(std::string);

	/**
	 * Retrieve the current database.
	 *
	 * @return The DB associated with this connection
	 */
	std::string GetDatabase();

	/**
	 * Change the active database for this connection
	 *
	 * @param db The new active database to use
	 * @throws A sql::SQLException for connection errors
	 */
	void SetDatabase(std::string db);
private:
	DbConnection();

	void Init(std::string connectionUrl, std::string username = "", std::string password = "", std::string db = "");

	DbConnectionInformation _connInfo;
	std::shared_ptr<sql::Connection> _connection;

	// Cached connection information
	DbConnectionInformation _oldInfo;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_DATABASE_DBCONNECTION_H_ */
