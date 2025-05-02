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
        msg = data['value']
        logger.info(f"Encoding message: {name} with ID: {id} from MessageFrame")
    else:
        msg = data
        id = get_msg_id(name)
        logger.info(f"Encoding message: {name} with ID: {id}")
    
    # Recursive function to preprocess data for bitstring fields
    def preprocess_bitstring(obj):
        if isinstance(obj, dict):
            # Recursively process dictionaries
            return {key: preprocess_bitstring(value) for key, value in obj.items()}
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
    # Recursive function to reverse bitstring fields
    def reverse_bitstring(obj):
        if isinstance(obj, dict):
            # Recursively process dictionaries
            return {key: reverse_bitstring(value) for key, value in obj.items()}
        elif isinstance(obj, tuple):
            if len(obj) == 2 and isinstance(obj[0], bytes) and isinstance(obj[1], int):
                # Convert (bytes, length) to [value, length]
                byte_value, length = obj
                binary_representation = bytes_to_binary(byte_value)  # Convert bytes to binary
                logger.debug(f"Binary representation: {binary_representation[0:length]}")  # Debugging output
                value = int(binary_representation[0:length], 2)  # Convert bytes back to an integer
                logger.debug(f"Value: {value}")
                return [value, length]
            elif len(obj) == 2 and isinstance(obj[0], str) and isinstance(obj[1], tuple):
                # Handle nested tuples with bitstring
                return [obj[0], reverse_bitstring(obj[1])]
            else:
                # Leave other tuples unchanged
                return [reverse_bitstring(item) for item in obj]
        elif isinstance(obj, bytes):
            # Handle octet strings (convert to hexadecimal for readability)
            hex_representation = obj.hex()
            logger.debug(f"Octet string (bytes): {obj}, Hex: {hex_representation}")
            return hex_representation  # Return as a hexadecimal string
        elif isinstance(obj, list):
            # Recursively process lists
            return [reverse_bitstring(item) for item in obj]
        else:
            return obj

    # Call the original decode method
    decoded_data = original_decode(name, msg)
    frame_dict = {'messageId': id, 'value': reverse_bitstring(decoded_data)}
    logger.info(f"Successfully decoded message!")
    logger.debug(f"Decoded data: {frame_dict}")

    # Reverse the bitstring fields in the decoded data
    return frame_dict

# Replace the original decode method with the custom one
J2735.decode = custom_decode