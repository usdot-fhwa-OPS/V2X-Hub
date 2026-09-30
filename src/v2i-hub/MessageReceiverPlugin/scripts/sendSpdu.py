import socket
import binascii as ba
import sys

udpIp = "127.0.0.1"
udpPort = 26789

txt = ("03806b0013680038422e1e7d2fc9ddd32f2e7971f4d3bf709b640800020d766174858008208214c8"
       "01011910c110c1002c0860853200304104299001021a2189a189806010c10c4c00a0820853200804"
       "644304430400d0218214c801c10410a6400c08688626862601c043043130")

count = int(sys.argv[1]) if len(sys.argv) > 1 else 1

unhexed = ba.unhexlify(txt)
sk = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
for _ in range(count):
    sk.sendto(unhexed, (udpIp, udpPort))
sk.close()