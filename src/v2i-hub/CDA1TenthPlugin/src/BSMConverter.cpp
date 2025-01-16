#include "BSMConverter.h"

namespace CDA1TenthPlugin
{
  pt BSMConverter::toTree(const BasicSafetyMessage_t &bsm)
  {
    // Logic to convert from input format to tree structure
    pt root;
    pt bsmChild;
    pt coreChild;
    coreChild.put("id", toTemporaryIdString(bsm.coreData.id));
    coreChild.put("msgCnt", bsm.coreData.msgCnt);
    coreChild.put("secMark", bsm.coreData.secMark);
    coreChild.put("lat", bsm.coreData.lat);
    coreChild.put("long", bsm.coreData.Long);
    coreChild.put("elev", bsm.coreData.elev);
    // // Add accurracy
    pt accuracyChild;
    accuracyChild.put("semiMajor", bsm.coreData.accuracy.semiMajor);
    accuracyChild.put("semiMinor", bsm.coreData.accuracy.semiMinor);
    accuracyChild.put("orientation", bsm.coreData.accuracy.orientation);
    coreChild.add_child("accuracy", accuracyChild);
    // Add transmission
    pt transmissionChild;
    transmissionChild.put(toTransmissionString(bsm.coreData.transmission), "");
    coreChild.add_child("transmission", transmissionChild);
    coreChild.put("speed", bsm.coreData.speed);
    coreChild.put("heading", bsm.coreData.heading);
    coreChild.put("angle", bsm.coreData.angle);
    // // Add accelSet
    pt accelSetChild;
    accelSetChild.put("yaw", bsm.coreData.accelSet.yaw);
    accelSetChild.put("long", bsm.coreData.accelSet.Long);
    accelSetChild.put("lat", bsm.coreData.accelSet.lat);
    accelSetChild.put("vert", bsm.coreData.accelSet.vert);
    coreChild.add_child("accelSet", accelSetChild);
    // // Add brakes
    pt brakesChild;
    brakesChild.put("wheelBrakes", toBrakeAppliedStatusString(bsm.coreData.brakes.wheelBrakes));
    // Add traction child
    pt tractionChild;
    tractionChild.put(toTractionString(bsm.coreData.brakes.traction), "");
    brakesChild.add_child("traction", tractionChild);
    // Add abs child
    pt absChild;
    absChild.put(toAntiLockBrakeStatusString(bsm.coreData.brakes.abs), "");
    brakesChild.add_child("abs", absChild);
    // Add scs child
    pt scsChild;
    scsChild.put(toStabilityControlStatusString(bsm.coreData.brakes.scs), "");
    brakesChild.add_child("scs", scsChild);
    // Add brakeBoost child
    pt brakeBoostChild;
    brakeBoostChild.put(toBrakeBoostAppliedString(bsm.coreData.brakes.brakeBoost), "");
    brakesChild.add_child("brakeBoost", brakeBoostChild);
    // Add auxBrakes child
    pt auxBrakesChild;
    auxBrakesChild.put(toAuxiliaryBrakeStatusString(bsm.coreData.brakes.auxBrakes), "");
    brakesChild.add_child("auxBrakes", auxBrakesChild);
    coreChild.add_child("brakes", brakesChild);
    // // Add size
    pt sizeChild;
    sizeChild.put("width", bsm.coreData.size.width);
    sizeChild.put("length", bsm.coreData.size.length);
    coreChild.add_child("size", sizeChild);
    bsmChild.add_child("coreData", coreChild);
    root.add_child("BasicSafetyMessage", bsmChild);
    return root;
  }

  string BSMConverter::toTransmissionString(const TransmissionState_t &transmission)
  {
    switch (transmission)
    {
    case TransmissionState::TransmissionState_neutral:
      return "neutral";
    case TransmissionState::TransmissionState_park:
      return "park";
    case TransmissionState::TransmissionState_forwardGears:
      return "forwardGears";
    case TransmissionState::TransmissionState_reverseGears:
      return "reverseGears";
    case TransmissionState::TransmissionState_reserved1:
      return "reserved1";
    case TransmissionState::TransmissionState_reserved2:
      return "reserved2";
    case TransmissionState::TransmissionState_reserved3:
      return "reserved3";
    case TransmissionState::TransmissionState_unavailable:
      return "unavailable";
    default:
      return "unknown";
    }
  }

  string BSMConverter::toTractionString(const TractionControlStatus_t &traction)
  {
    switch (traction)
    {
    case TractionControlStatus::TractionControlStatus_unavailable:
      return "unavailable";
    case TractionControlStatus::TractionControlStatus_off:
      return "off";
    case TractionControlStatus::TractionControlStatus_on:
      return "on";
    case TractionControlStatus::TractionControlStatus_engaged:
      return "engaged";
    default:
      return "unknown";
    }
  }

  string BSMConverter::toAntiLockBrakeStatusString(const AntiLockBrakeStatus_t &abs)
  {
    switch (abs)
    {
    case AntiLockBrakeStatus::AntiLockBrakeStatus_unavailable:
      return "unavailable";
    case AntiLockBrakeStatus::AntiLockBrakeStatus_off:
      return "off";
    case AntiLockBrakeStatus::AntiLockBrakeStatus_on:
      return "on";
    case AntiLockBrakeStatus::AntiLockBrakeStatus_engaged:
      return "engaged";
    default:
      return "unknown";
    }
  }

  string BSMConverter::toStabilityControlStatusString(const StabilityControlStatus_t &scs)
  {
    switch (scs)
    {
    case StabilityControlStatus::StabilityControlStatus_unavailable:
      return "unavailable";
    case StabilityControlStatus::StabilityControlStatus_off:
      return "off";
    case StabilityControlStatus::StabilityControlStatus_on:
      return "on";
    case StabilityControlStatus::StabilityControlStatus_engaged:
      return "engaged";
    default:
      return "unknown";
    }
  }

  string BSMConverter::toBrakeBoostAppliedString(const BrakeBoostApplied_t &brakeBoost)
  {
    switch (brakeBoost)
    {
    case BrakeBoostApplied::BrakeBoostApplied_unavailable:
      return "unavailable";
    case BrakeBoostApplied::BrakeBoostApplied_off:
      return "off";
    case BrakeBoostApplied::BrakeBoostApplied_on:
      return "on";
    default:
      return "unknown";
    }
  }

  string BSMConverter::toAuxiliaryBrakeStatusString(const AuxiliaryBrakeStatus_t &auxBrakes)
  {
    switch (auxBrakes)
    {
    case AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_unavailable:
      return "unavailable";
    case AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_off:
      return "off";
    case AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_on:
      return "on";
    case AuxiliaryBrakeStatus::AuxiliaryBrakeStatus_reserved:
      return "reserved";
    default:
      return "unknown";
    }
  }

  string BSMConverter::toBrakeAppliedStatusString(const BrakeAppliedStatus_t &wheelBrakes)
  {
    std::string result;
    for (int i = 0; i < 5; ++i)
    {
      result += (wheelBrakes.buf[i / 8] & (1 << (7 - (i % 8)))) ? '1' : '0';
    }
    return result;
  }

  string BSMConverter::toTemporaryIdString(const TemporaryID_t &id)
  {
    std::stringstream id_fill_ss;
    for (int i = 0; i < id.size; i++)
    {
      id_fill_ss << std::setfill('0') << std::setw(2) << std::hex << (int)id.buf[i];
    }
    return id_fill_ss.str();
  }

  string BSMConverter::toJsonString(const pt &tree)
  {
    std::ostringstream buf;
    write_json(buf, tree, false);
    std::string jsonStr = buf.str();
    boost::algorithm::erase_all(jsonStr, "\n");
    boost::algorithm::erase_all(jsonStr, "\t");
    boost::algorithm::erase_all(jsonStr, " ");
    return jsonStr;
    return buf.str();
  }
}