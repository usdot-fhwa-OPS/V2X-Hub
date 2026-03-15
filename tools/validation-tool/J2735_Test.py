from J2735 import J2735
import json

msg_str = '00142a4049dc64224d21266e98fa1ea6d05580007ffffffff0007080fdfa1fa1007fffa000731038080d001e00'
# Decode the encoded msg
decoded_frame = J2735.decode(msg_str)

print(f"Decoded Frame: {decoded_frame}")

with open('lge_example.json', 'r') as f:
    Loaded_msg = json.load(f)

# Encode the decoded msg
hex_frame = J2735.encode(Loaded_msg)

print(f"Encoded Frame: {hex_frame}")

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

