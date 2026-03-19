#!/usr/bin/env python3
"""
Auto-detect directed-broadcast address and broadcast the BSM payload.
Works by parsing `ip -4 addr` output to find the first non-loopback interface
with an IPv4 address and its 'brd' value.
"""

import subprocess, re, socket, binascii as ba, time, sys

# ---- payload + settings ----
HEX_PAYLOAD = ("001480CF4B950C400022D2666E923D1EA6D4E28957BD55FFFFF001C758FD7E67D07F7FFF8000000002020218E1C1004A40196FBC042210115C030EF1408801021D4074CE7E1848101C5C0806E8E1A50101A84056EE8A1AB4102B840A9ADA21B9010259C08DEE1C1C560FFDDBFC070C0222210018BFCE309623120FFE9BFBB10C8238A0FFDC3F987114241610009BFB7113024780FFAC3F95F13A26800FED93FDD51202C5E0FE17BF9B31202FBAFFFEC87FC011650090019C70808440C83207873800000000001095084081C903447E31C12FC0")
UDP_PORT = 26789
SEND_RATE_HZ = 10
IP_CMD = ["ip", "-4", "addr"]

def get_broadcast_from_ip_output():
    try:
        out = subprocess.check_output(IP_CMD, universal_newlines=True)
    except Exception as e:
        return None, None

    # Split into blocks per interface
    blocks = re.split(r"\n\d+:", out)
    for b in blocks:
        if "inet " in b and " lo " not in b:
            # find inet line, e.g.: "    inet 192.168.1.42/24 brd 192.168.1.255 scope global enp3s0"
            m = re.search(r"\s+inet\s+([0-9.]+\)/\d+)\s+brd\s+([0-9.]+)\s+.*", b)
            if not m:
                # alternate pattern
                m2 = re.search(r"\s+inet\s+([0-9.]+)/\d+\s+brd\s+([0-9.]+)", b)
                if m2:
                    ipaddr = m2.group(1)
                    brd = m2.group(2)
                    # find interface name
                    ifname_m = re.search(r"^\s*([^\s:]+)", b.strip())
                    ifname = ifname_m.group(1) if ifname_m else None
                    return ifname, brd
            else:
                # fallback (shouldn't happen)
                pass
    return None, None

def main():
    try:
        payload = ba.unhexlify(HEX_PAYLOAD)
    except Exception as e:
        print("Invalid HEX payload:", e, file=sys.stderr)
        sys.exit(1)

    ifname, brd = get_broadcast_from_ip_output()
    if brd:
        BROADCAST_IP = brd
        print(f"Detected interface {ifname} with broadcast {BROADCAST_IP}")
    else:
        BROADCAST_IP = "255.255.255.255"
        print("Warning: could not auto-detect broadcast. Falling back to 255.255.255.255")

    sk = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sk.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sk.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    interval = 1.0 / SEND_RATE_HZ
    print(f"Broadcasting {len(payload)} bytes to {BROADCAST_IP}:{UDP_PORT} at {SEND_RATE_HZ} Hz. Ctrl-C to stop.")
    try:
        while True:
            sk.sendto(payload, (BROADCAST_IP, UDP_PORT))
            time.sleep(interval)
    except KeyboardInterrupt:
        print("Stopped.")
    finally:
        sk.close()

if __name__ == "__main__":
    main()