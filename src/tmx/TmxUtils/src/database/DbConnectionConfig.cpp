/*
 * @file DbConnectionConfig.cpp
 *
 *  Created on: 2026-02-18
 *      @author: Daniel I. Kelley (Leidos)
 */

#include "DbConnectionConfig.h"


namespace tmx::utils{

// Default values for backward compatibility
const std::string DbConnectionConfig::DEFAULT_HOST = "127.0.0.1";
const std::string DbConnectionConfig::DEFAULT_PORT = "3306";
const std::string DbConnectionConfig::DEFAULT_DATABASE = "IVP";
const std::string DbConnectionConfig::DEFAULT_USER = "IVP";

DbConnectionConfig::DbConnectionConfig() {
    loadFromEnvironment();
}

DbConnectionConfig& DbConnectionConfig::getInstance() {
    static DbConnectionConfig instance;
    return instance;
}

void DbConnectionConfig::loadFromEnvironment() {
    std::scoped_lock lock(configMutex);
    
    host = getEnvVar("MYSQL_HOST", DEFAULT_HOST);
    port = getEnvVar("MYSQL_PORT", DEFAULT_PORT);
    database = getEnvVar("MYSQL_DATABASE", DEFAULT_DATABASE);
    user = getEnvVar("MYSQL_USER", DEFAULT_USER);

    
    // Get password file path from environment
    const char* pwdFileEnv = std::getenv("MYSQL_PASSWORD");
    if (pwdFileEnv != nullptr) {
        passwordFile = std::string(pwdFileEnv);
    } else {
        passwordFile = "";
        FILE_LOG(logWARNING) << "MYSQL_PASSWORD environment variable not set" << std::endl;
    }
    
    FILE_LOG(logDEBUG) << "Database configuration loaded - Host: " << host 
                   << ", Port: " << port 
                   << ", Database: " << database 
                   << ", User: " << user << std::endl;
}

std::string DbConnectionConfig::getEnvVar(const char* envVar, const std::string& defaultValue) const {
    const char* value = std::getenv(envVar);
    if (value != nullptr) {
        std::string_view sv(value);
        if (!sv.empty()) return std::string(sv);
    }
    return defaultValue;
}

std::string DbConnectionConfig::getConnectionUrl() const {
    std::scoped_lock lock(configMutex);
    std::ostringstream url;
    url << "tcp://" << host << ":" << port;
    return url.str();
}

std::string DbConnectionConfig::getHost() const {
    std::scoped_lock lock(configMutex);
    return host;
}

std::string DbConnectionConfig::getPort() const {
    std::scoped_lock lock(configMutex);
    return port;
}

std::string DbConnectionConfig::getDatabase() const {
    std::scoped_lock lock(configMutex);
    return database;
}

std::string DbConnectionConfig::getUser() const {
    std::scoped_lock lock(configMutex);
    return user;
}

std::string DbConnectionConfig::getPassword() const {
    std::scoped_lock lock(configMutex);
    std::string password;

    if (passwordFile.empty()) {
        throw DbConnectionException( "MYSQL_PASSWORD environment variable not set"); 
    }
    try {
        const std::filesystem::path filepath = std::filesystem::canonical(passwordFile);

    
        std::ifstream file(passwordFile);
        if (!file.is_open()) {
            FILE_LOG(logERROR) << "Unable to read password file: " << passwordFile << std::endl;
            return "";
        }
    
        std::getline(file, password);
    }
    catch (const std::filesystem::filesystem_error &e) {
        throw DbConnectionException(e);
    }
    
    if (password.empty()) {
        throw DbConnectionException("Empty MYSQL_PASSWORD secret!");
    }
    return password;
}

void DbConnectionConfig::reloadConfiguration() {
    FILE_LOG(logINFO) << "Reloading database configuration from environment variables" << std::endl;
    loadFromEnvironment();
}

}
