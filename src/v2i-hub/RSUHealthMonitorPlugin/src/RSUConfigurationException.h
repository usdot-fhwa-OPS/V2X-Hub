#pragma once

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::runtime_error
    {
    public:
        explicit RSUConfigurationException(const std::string &msg) : std::runtime_error(msg){};
    };
}