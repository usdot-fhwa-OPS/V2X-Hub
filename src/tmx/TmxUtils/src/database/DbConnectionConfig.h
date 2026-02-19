/*
 * @file DbConnectionConfig.h
 *
 *  Created on: 2026-02-18
 *      @author: Daniel I. Kelley (Leidos)
 */

#ifndef SRC_DATABASE_DBCONNECTIONCONFIG_H_
#define SRC_DATABASE_DBCONNECTIONCONFIG_H_

#include <string>
#include <mutex>

namespace tmx {
namespace utils {

/**
 * A singleton class to manage database connection configuration through environment variables.
 * This class provides a centralized way to access database connection parameters,
 * eliminating hardcoded database connection strings throughout the codebase.
 */
class DbConnectionConfig {
public:
    /**
     * Get the singleton instance of DbConnectionConfig
     * @return Reference to the singleton instance
     */
    static DbConnectionConfig& getInstance();

    /**
     * Get the complete database connection URL
     * @return Connection URL in format "tcp://host:port"
     */
    std::string getConnectionUrl() const;

    /**
     * Get the database host
     * @return Database host (default: "127.0.0.1" or value from MYSQL_HOST)
     */
    std::string getHost() const;

    /**
     * Get the database port
     * @return Database port (default: "3306" or value from MYSQL_PORT)
     */
    std::string getPort() const;

    /**
     * Get the database name
     * @return Database name (default: "IVP" or value from MYSQL_DATABASE)
     */
    std::string getDatabase() const;

    /**
     * Get the database user
     * @return Database user (default: "IVP" or value from MYSQL_USER)
     */
    std::string getUser() const;

    /**
     * Get the database password from file
     * @return Database password read from file specified by MYSQL_PASSWORD environment variable
     */
    std::string getPassword() const;

    /**
     * Reload configuration from environment variables
     * This can be called to refresh configuration if environment changes
     */
    void reloadConfiguration();

private:
    DbConnectionConfig();
    ~DbConnectionConfig() = default;
    
    // Prevent copying
    DbConnectionConfig(const DbConnectionConfig&) = delete;
    DbConnectionConfig& operator=(const DbConnectionConfig&) = delete;

    /**
     * Load configuration from environment variables
     */
    void loadFromEnvironment();

    /**
     * Get environment variable value with default fallback
     * @param envVar Environment variable name
     * @param defaultValue Default value if environment variable is not set
     * @return Environment variable value or default
     */
    std::string getEnvVar(const char* envVar, const std::string& defaultValue) const;

    // Configuration values
    std::string host;
    std::string port;
    std::string database;
    std::string user;
    std::string passwordFile;
    
    // Thread safety
    mutable std::mutex configMutex;
    
    // Default values for backward compatibility
    static const std::string DEFAULT_HOST;
    static const std::string DEFAULT_PORT;
    static const std::string DEFAULT_DATABASE;
    static const std::string DEFAULT_USER;
};

} /* namespace utils */
} /* namespace tmx */

#endif /* SRC_DATABASE_DBCONNECTIONCONFIG_H_ */
