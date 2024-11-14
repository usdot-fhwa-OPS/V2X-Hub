#pragma once
#include <tmx/TmxException.hpp>

namespace tmx::utils::telemetry{
    class TelemetryDeserializerException: public tmx::TmxException
    {
    public:
        using tmx::TmxException::TmxException;
        ~TelemetryDeserializerException() override =default;
    };
}
