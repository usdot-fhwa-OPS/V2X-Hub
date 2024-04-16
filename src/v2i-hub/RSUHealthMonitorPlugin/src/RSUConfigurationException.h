#pragma once
#include <iostream>

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::runtime_error
    {
    public:
        RSUConfigurationException(const char *msg) : runtime_error(msg){};
    };
}