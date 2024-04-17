#pragma once

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };
}