#pragma once
#include <iostream>

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::runtime_error
    {
    private:
        std::string message;

    public:
        RSUConfigurationException(const char *msg) : runtime_error(msg){};
    };

}