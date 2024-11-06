#pragma once
#include <tmx/TmxException.hpp>

namespace tmx::utils::telemetry{
    class TelemetrySerializerException: public tmx::TmxException
    {
    public:
        explicit TelemetrySerializerException(const std::string& errorMessage): tmx::TmxException(errorMessage){};
        ~TelemetrySerializerException()=default;
    };
}
