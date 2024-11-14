#pragma once
#include <tmx/TmxException.hpp>

namespace tmx::utils::telemetry{
    class TelemetrySerializerException: public tmx::TmxException
    {
    public:
        using tmx::TmxException::TmxException;
        ~TelemetrySerializerException() override = default;
    };
}
