#pragma once
#include <tmx/TmxException.hpp>

namespace TelematicBridge
{
    class TelematicBridgeException : public tmx::TmxException
    {
    public:
       using TmxException::TmxException;
    };
}