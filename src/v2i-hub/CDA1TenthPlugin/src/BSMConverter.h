#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <tmx/j2735_messages/BasicSafetyMessage.hpp>

using namespace tmx::messages;
using pt = boost::property_tree::ptree;
using std::string;
namespace CDA1TenthPlugin
{
  class BSMConverter
  {
  public:
    BSMConverter() = default;
    ~BSMConverter() = default;
    static pt toTree(const BasicSafetyMessage_t &bsm);
    static string toJsonString(const pt &tree);
    static string toTemporaryIdString(const TemporaryID_t &id);
    static string toTransmissionString(const TransmissionState_t &transmission);
    static string toTractionString(const TractionControlStatus_t &traction);
    static string toBrakeAppliedStatusString(const BrakeAppliedStatus_t &wheel);
    static string toBrakeSystemStatusString(const AntiLockBrakeStatus_t &abs);
    static string toStabilityControlStatusString(const StabilityControlStatus_t &scs);
    static string toBrakeBoostAppliedString(const BrakeBoostApplied_t &brakeBoost);
    static string toAuxiliaryBrakeStatusString(const AuxiliaryBrakeStatus_t &auxBrakes);
  };
} // namespace CDA1TenthPlugin
