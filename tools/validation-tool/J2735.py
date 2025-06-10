# Auto-generated Python module from ASN.1 files
# Do not edit manually

import pickle
import logging
from binascii import hexlify, unhexlify

# Parameters
version = '2024'
log_level = logging.ERROR

# Set up logging
logger = logging.getLogger(__name__)
logger.setLevel(log_level)

# J2735 Message ID <-> Type mapping
msgs = {
    18:'MapData',
    19:'SPAT',
    20:'BasicSafetyMessage',
    21:'CommonSafetyRequest',
    22:'EmergencyVehicleAlert',
    23:'IntersectionCollision',
    24:'NMEAcorrections',
    25:'ProbeDataManagement',
    26:'probeVehicleData',
    27:'RoadSideAlert',
    28:'RTCMcorrections',
    29:'SignalRequestMessage',
    30:'SignalStatusMessage',
    31:'TravelerInformation',
    32:'PersonalSafetyMessage',
    33:'RoadSafetyMessage',
    34:'RoadWeatherMessage',
    35:'ProbeDataConfig',
    36:'ProbeDataReport',
    37:'TollAdvertisementMessage',
    38:'TollUsageMessage',
    39:'TollUsageAckMessage',
    40:'CooperativeControlMessage',
    41:'SensorDataSharingMessage',
    42:'ManeuverSharingAndCoordinatingMessage',
    43:'RoadGeometryAndAttributes',
    44:'PersonalSafetyMessage2',
    45:'TrafficSignalPhaseAndTiming',
    46:'SignalControlAndPrioritizationRequest',
    47:'SignalControlAndPrioritizationStatus',
    48:'RoadUserChargingConfigMessage',
    49:'RoadUserChargingReportMessage',
    50:'TrafficLightStatusMessage'
}
BSM_Part_II_mapping = {
    0: 'VehicleSafetyExtensions',
    1: 'SpecialVehicleExtensions',
    2: 'SupplementalVehicleExtensions'
}
# Reverse mapping
ids = {v: k for k, v in msgs.items()}

# Get the message name from the message ID
def get_msg_name(msg_id):
    try:
        return msgs.get(msg_id)
    except KeyError:
        logger.error(f"Message ID '{msg_id}' is not supported.")
        exit(1)
# Get the message ID from the message Name
def get_msg_id(msg_name):
    try:
        return ids.get(msg_name)
    except KeyError:
        logger.error(f"Message Type '{msg_name}' is not supported.")
        exit(1)


# This function loads the compiled J2735 module from a pickle file.
def get_compiled(version='2020'):
    with open(f'tools/validation-tool/J2735_Compiled/J2735_{version}.pkl', 'rb') as f:
        loaded = pickle.load(f)
    return loaded

# Load the compiled J2735 module based on the version
J2735 = get_compiled(version)

# Monkey patch the encode method to handle bitstring conversion
original_encode = J2735.encode  # Save the original encode method

def custom_encode(data, name='MessageFrame'):
    
    if 'messageId' in data:
        id = data['messageId']
        name = get_msg_name(id)
        msg = data['value'] if isinstance(data['value'], dict) else data['value'][1]
        logger.info(f"Encoding message: {name} with ID: {id} from MessageFrame")
    else:
        msg = data
        id = get_msg_id(name)
        logger.info(f"Encoding message: {name} with ID: {id}")
    
    # Recursive function to preprocess data for bitstring fields
    def preprocess_bitstring(obj, parent_name=None):
        # Special handling for BSM Part II
        if parent_name == "partII" and isinstance(obj, list):
            processed_list = []
            for entry in obj:
                if (
                    isinstance(entry, dict)
                    and "partII-Id" in entry
                    and "partII-Value" in entry
                ):
                    partii_id = entry["partII-Id"]
                    partii_type = BSM_Part_II_mapping.get(partii_id)
                    partii_value = entry["partII-Value"] if isinstance(entry["partII-Value"], dict) else entry["partII-Value"][1]
                    # If the value is a dict (decoded), encode it using the correct type
                    if partii_type and isinstance(partii_value, dict):
                        encoded_value = original_encode(partii_type, preprocess_bitstring(partii_value, partii_type))
                        entry = entry.copy()
                        entry["partII-Value"] = encoded_value
                processed_list.append(preprocess_bitstring(entry))
            return processed_list

        if isinstance(obj, dict):
            return {key: preprocess_bitstring(value, key) for key, value in obj.items()}
        elif isinstance(obj, list):
            if len(obj) == 2 and all(isinstance(i, int) for i in obj):
                # Convert [value, length] to (bytes, length)
                value, length = obj
                byte_length = (length + 7) // 8  # Calculate the number of bytes needed
                # Convert the integer to a binary string
                binary_string = f'{value:b}'.zfill(length)  # Binary string without leading zeros
                # Add trailing zeros to match the required length
                binary_string = binary_string.ljust(byte_length*8, '0')
                # Convert the binary string to bytes
                byte_value = int(binary_string, 2).to_bytes(byte_length, byteorder='big')
                return (byte_value, length)
            elif len(obj) == 2 and isinstance(obj[0], str):# and isinstance(obj[1], list):
                # Handle nested lists with bitstring
                return (obj[0], preprocess_bitstring(obj[1]))
            else:
                # Leave other lists unchanged
                return [preprocess_bitstring(item) for item in obj]
        elif isinstance(obj, str):
            try:
                # Attempt to decode the string as a hexadecimal value
                return bytes.fromhex(obj)
            except ValueError:
                # If decoding fails, return the string as is
                return obj
        else:
            return obj

    # Preprocess the data to handle bitstring fields
    processed_data = preprocess_bitstring(msg)
    # encode the message
    encoded_msg = original_encode(name, processed_data)
    # encode message frame
    encoded_frame = original_encode('MessageFrame', {'messageId': id, 'value': encoded_msg})
    logger.info(f"Successfully encoded message!")
    logger.debug(f"Encoded data: {hexlify(encoded_frame).decode('utf-8')}")

    # Call the original encode method with the processed data
    return hexlify(encoded_frame).decode('utf-8')

# Replace the original encode method with the custom one
J2735.encode = custom_encode

# Monkey patch the decode method to reverse bitstring conversion
original_decode = J2735.decode  # Save the original decode method

def bytes_to_binary(byte_value):
    return ''.join(f'{byte:08b}' for byte in byte_value)

def custom_decode(data, name='MessageFrame'):
    if name == 'MessageFrame':
        # Decode the message frame
        decoded_frame = original_decode(name, unhexlify(data))
        id = decoded_frame['messageId']
        msg = decoded_frame['value']
        name = get_msg_name(id)
        logger.info(f"Decoding message: {name} with ID: {id} from MessageFrame")
    else:
        id = get_msg_id(name)
        msg = unhexlify(data)
        logger.info(f"Decoding message: {name} with ID: {id}")

    def reverse_bitstring(obj, parent_name=None):
        # Special handling for BSM Part II
        if parent_name == "partII" and isinstance(obj, list):
            # Each item is a dict with partII-Id and partII-Value
            decoded_list = []
            for entry in obj:
                if (
                    isinstance(entry, dict)
                    and "partII-Id" in entry
                    and "partII-Value" in entry
                ):
                    partii_id = entry["partII-Id"]
                    partii_type = BSM_Part_II_mapping.get(partii_id)
                    partii_value = entry["partII-Value"]
                    if partii_type and isinstance(partii_value, bytes):
                        # Decode the partII-Value using the correct type
                        decoded_value = original_decode(partii_type, partii_value)
                        decoded_value = reverse_bitstring(decoded_value, partii_type)
                        entry["partII-Value"] = [partii_type, decoded_value]
                decoded_list.append(reverse_bitstring(entry))
            return decoded_list

        if isinstance(obj, dict):
            return {
                key: reverse_bitstring(value, key)
                for key, value in obj.items()
            }
        elif isinstance(obj, tuple):
            if len(obj) == 2 and isinstance(obj[0], bytes) and isinstance(obj[1], int):
                byte_value, length = obj
                binary_representation = bytes_to_binary(byte_value)
                logger.debug(f"Binary representation: {binary_representation[0:length]}")
                value = int(binary_representation[0:length], 2)
                logger.debug(f"Value: {value}")
                return [value, length]
            elif len(obj) == 2 and isinstance(obj[0], str) and isinstance(obj[1], tuple):
                return [obj[0], reverse_bitstring(obj[1])]
            else:
                return [reverse_bitstring(item) for item in obj]
        elif isinstance(obj, bytes):
            hex_representation = obj.hex()
            logger.debug(f"Octet string (bytes): {obj}, Hex: {hex_representation}")
            return hex_representation
        elif isinstance(obj, list):
            return [reverse_bitstring(item, parent_name) for item in obj]
        else:
            return obj

    # Call the original decode method
    decoded_data = original_decode(name, msg)
    frame_dict = {'messageId': id, 'value': [name, reverse_bitstring(decoded_data, name)]}
    logger.info(f"Successfully decoded message!")
    logger.debug(f"Decoded data: {frame_dict}")

    # Reverse the bitstring fields in the decoded data
    return frame_dict

# Replace the original decode method with the custom one
J2735.decode = custom_decode