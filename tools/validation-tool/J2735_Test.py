from J2735 import J2735

msg_str = '001374003842BE5E7D1049DDD32F2E7971F4D3BF7097B4080033C76C31660580082080BF7000810D0602E627242C08202FDC0080200608202FDC0040434182498B590C02080C124020080282080BF7001810D0602E627243408202FDC0080200E08683049316B00804341824990090E02080C124020080'

# Decode the encoded msg
decoded_frame = J2735.decode(msg_str)

print(f"Decoded Frame: {decoded_frame}")

# Encode the decoded msg
hex_frame = J2735.encode(decoded_frame)

# print(f"Encoded Frame: {hex_frame}")

# Check similarity
success = msg_str.upper() == hex_frame.upper()
print(f"success: {success}")

# If the encoded frame does not match the original message string, print the differences
if not success:
    print(f"Original: {msg_str}")
    print(f"Encoded:  {hex_frame}")
    for i, c in enumerate(hex_frame.upper()):
        k = msg_str.upper()[i]
        if c != k:
            print(f"diff @ {i}: {c} != {k}")

