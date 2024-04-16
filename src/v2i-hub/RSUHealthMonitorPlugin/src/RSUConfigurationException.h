#pragma once
#include <iostream>

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::exception
    {
    private:
        std::string message;

    public:
        RSUConfigurationException(const char *msg) : message(msg){};
        const char *what()
        {
            return message.c_str();
        }
    };

}