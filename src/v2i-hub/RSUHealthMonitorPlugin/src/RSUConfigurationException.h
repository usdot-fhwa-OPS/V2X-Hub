#pragma once

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::runtime_error
    {
    public:
        RSUConfigurationException(const std::string &msg) : std::runtime_error(msg){};
    };
}