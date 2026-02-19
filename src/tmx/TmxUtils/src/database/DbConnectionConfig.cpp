/*
 * @file DbConnectionConfig.cpp
 *
 *  Created on: 2026-02-18
 *      @author: Daniel I. Kelley (Leidos)
 */

#include "DbConnectionConfig.h"
#include "../PluginLog.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace tmx {
namespace utils {

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
    std::lock_guard<std::mutex> lock(configMutex);
    
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
        PLOG(logWARNING) << "MYSQL_PASSWORD environment variable not set";
    }
    
    PLOG(logDEBUG) << "Database configuration loaded - Host: " << host 
                   << ", Port: " << port 
                   << ", Database: " << database 
                   << ", User: " << user;
}

std::string DbConnectionConfig::getEnvVar(const char* envVar, const std::string& defaultValue) const {
    const char* value = std::getenv(envVar);
    if (value != nullptr && strlen(value) > 0) {
        return std::string(value);
    }
    return defaultValue;
}

std::string DbConnectionConfig::getConnectionUrl() const {
    std::lock_guard<std::mutex> lock(configMutex);
    std::ostringstream url;
    url << "tcp://" << host << ":" << port;
    return url.str();
}

std::string DbConnectionConfig::getHost() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return host;
}

std::string DbConnectionConfig::getPort() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return port;
}

std::string DbConnectionConfig::getDatabase() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return database;
}

std::string DbConnectionConfig::getUser() const {
    std::lock_guard<std::mutex> lock(configMutex);
    return user;
}

std::string DbConnectionConfig::getPassword() const {
    std::lock_guard<std::mutex> lock(configMutex);
    
    if (passwordFile.empty()) {
        PLOG(logERROR) << "MYSQL_PASSWORD environment variable not set";
        return "";
    }
    
    std::ifstream file(passwordFile);
    if (!file.is_open()) {
        PLOG(logERROR) << "Unable to read password file: " << passwordFile;
        return "";
    }
    
    std::string password;
    std::getline(file, password);
    
    if (password.empty()) {
        PLOG(logERROR) << "Empty password file: " << passwordFile;
        return "";
    }
    
    return password;
}

void DbConnectionConfig::reloadConfiguration() {
    PLOG(logINFO) << "Reloading database configuration from environment variables";
    loadFromEnvironment();
}

} /* namespace utils */
} /* namespace tmx */
