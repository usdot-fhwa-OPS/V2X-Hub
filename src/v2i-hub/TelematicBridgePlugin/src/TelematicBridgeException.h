#pragma once
#include <tmx/TmxException.hpp>

namespace TelematicBridge
{
    class TelematicBridgeException : public tmx::TmxException
    {
    public:
        explicit TelematicBridgeException(const std::string &what_arg) : TmxException(what_arg)
        {
        }

        explicit TelematicBridgeException(const char *what_arg = "") : TmxException(what_arg)
        {
        }
        ~TelematicBridgeException() = default;
    };
}