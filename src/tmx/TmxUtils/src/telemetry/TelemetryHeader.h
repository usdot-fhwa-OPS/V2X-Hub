#pragma once
#include <string>

using std::string;
namespace tmx::utils::telemetry{
    struct TelemetryHeader
    {
        string type;
        string subtype;
        string encoding;
        uint64_t timestamp;
        string flags;
    };
}
