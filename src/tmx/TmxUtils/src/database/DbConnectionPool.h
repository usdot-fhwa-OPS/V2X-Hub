/*
 * @file DbConnectionPool.h
 *
 *  Created on: Aug 5, 2016
 *      @author: Gregory M. Baumgardner
 */

#ifndef SRC_DATABASE_DBCONNECTIONPOOL_H_
#define SRC_DATABASE_DBCONNECTIONPOOL_H_

#include "DbConnection.h"

#include <cppconn/connection.h>
#include <memory>
#include <vector>

#ifndef MAX_DATABASE_CONNECTIONS
#define MAX_DATABASE_CONNECTIONS 10
#endif

namespace tmx {
namespace utils {

/**
 * A class to access a shared pool of database connections.  Using this class ensures that connections to the database
 * are kept open as long as possible, and opened ones are re-used, in order to achieve optimal performance for database
 * access.
 */
class DbConnectionPool {
public:
	/**
	 * Create a connection pool object.  Note that the actual backend container is static, thus the connections are shared
	 * amongst the entire process space.
	 */
	DbConnectionPool();

	/**
	 * Get a connection from the pool.
	 *
	 * @param connectionUrl The URL to connect to
	 * @param username The user to login with, or the default
	 * @param password The password to login with, or the default
	 * @param db The active database to use, or none
	 * @throws A sql::SQLException for connection errors
	 * @return An open DbConnection object
	 * @see DbConnection(string, string, string, string)
	 */
	DbConnection Connection(std::string connectionUrl = "", std::string username = "", std::string password = "", std::string db = "");

	/**
	 * Close all unused connections in the pool.  Since the normal operation is to keep the connection open for future re-use, this
	 * function should be rarely used.
	 */
	void Release();

	/**
	 * @return The current size of the pool
	 */
	size_t Size();

	/**
	 * @return The maximum size of the pool
	 */
	size_t MaxSize();

	/**
	 * @return The number of connections in the pool that are currently in use
	 */
	size_t ActiveSize();

	/**
	 * @return The number of database connections in this pool.
	 */
	size_t NumConnections();

	void SetConnectionUrl(std::string connectionUrl);
private:
	static std::vector<DbConnection> pool;
	std::string _connectStr;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_DATABASE_DBCONNECTIONPOOL_H_ */
