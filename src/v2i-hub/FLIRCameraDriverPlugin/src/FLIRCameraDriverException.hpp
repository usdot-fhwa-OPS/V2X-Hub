#pragma once
#include <tmx/TmxException.hpp>

namespace FLIRCameraDriverPlugin
{
    class FLIRCameraDriverException : public tmx::TmxException
    {
    public:
       using TmxException::TmxException;
    };
}
