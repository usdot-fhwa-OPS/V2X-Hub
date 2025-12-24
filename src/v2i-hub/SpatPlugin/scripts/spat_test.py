# Sends either a larger payload or a binary file to the SPAT Plugin to test TSCBM overflow handling
#!/usr/bin/env python3
import socket
import time
import os

TARGET_IP = "127.0.0.1"

TARGET_PORT = 6053
PAYLOAD_SIZE = 500
# SPaT binary file path
FILE_PATH = "../../SpatPlugin/test/test_spat_binaries/spat_1721238398773.bin"

# Overflow Payload Sender Function
def send_overflow_payload():
    """Send a larger UDP payload to test TSCBM overflow handling."""
    time.sleep(1)
    payload = b"X" * PAYLOAD_SIZE

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.sendto(payload, (TARGET_IP, TARGET_PORT))

    print(f"[Overflow Test] Sent {PAYLOAD_SIZE} bytes to {TARGET_IP}:{TARGET_PORT}")

# Binary File Sender Function
def send_binary_file():
    """Send a test SPaT binary message over UDP."""
    if not os.path.exists(FILE_PATH):
        raise FileNotFoundError(f"Binary file not found: {FILE_PATH}")

    with open(FILE_PATH, "rb") as f:
        payload = f.read()

    print(f"[Binary File] Loaded {len(payload)} bytes from file: {FILE_PATH}")

    time.sleep(1)

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.sendto(payload, (TARGET_IP, TARGET_PORT))

    print(f"[Binary File] Sent {len(payload)} bytes to {TARGET_IP}:{TARGET_PORT}")

def main():
    print("\n=== SPaT Payload Test Menu ===")
    print("1. Send overflow payload")
    print("2. Send binary SPaT file")
    print("3. Exit")

    choice = input("Select an option (1/2/3): ").strip()

    if choice == "1":
        send_overflow_payload()
    elif choice == "2":
        send_binary_file()
    elif choice == "3":
        print("Exiting.")
    else:
        print("Invalid choice. Please run again.")

if __name__ == "__main__":
    main()