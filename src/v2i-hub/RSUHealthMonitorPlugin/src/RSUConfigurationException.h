#pragma once
#include <iostream>

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::runtime_error
    {
    public:
        explicit RSUConfigurationException(const char *msg) : runtime_error(msg){};
    };
}