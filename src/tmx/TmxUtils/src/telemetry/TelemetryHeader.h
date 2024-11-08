#pragma once
#include <string>

using namespace std;
namespace tmx::utils::telemetry{
    struct TelemetryHeader
    {
        string type;
        string subtype;
        string encoding;
        uint64_t timestamp;
    };
}
