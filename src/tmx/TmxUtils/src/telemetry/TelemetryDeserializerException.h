#pragma once
#include <tmx/TmxException.hpp>

namespace tmx::utils::telemetry{
    class TelemetryDeserializerException: public tmx::TmxException
    {
    public:
        explicit TelemetryDeserializerException(const std::string& errorMessage): tmx::TmxException(errorMessage){};
        ~TelemetryDeserializerException()=default;
    };
}
