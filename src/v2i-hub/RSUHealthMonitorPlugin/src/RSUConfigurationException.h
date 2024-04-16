#pragma once
#include <tmx/TmxException.hpp>

namespace RSUHealthMonitor
{
    class RSUConfigurationException : public tmx::TmxException
    {
    public:
        using TmxException::TmxException;
    };
}