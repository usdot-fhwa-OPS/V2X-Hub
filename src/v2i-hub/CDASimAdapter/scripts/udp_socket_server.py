import socket
'''
Purpose: This script is to launch a UDP server and used to test the CDASimAdapter simulation registration.
Usage: Run this script first, it shall open a socket listenning to port 6767.
    Launch the v2xhub and enable the CDASimAdapter plugin. Upon the plugin startup,
    it sends registration message to port 6767. This UDP server shall receive the registration message,
    and print this message on the terminal.
Run command: python3 udp_socket_server.py
'''
UDP_IP = "127.0.0.1"
UDP_SOCKET_PORT_SIM_REGISTRATION = 6767

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_SOCKET_PORT_SIM_REGISTRATION))
print("Server Listenning on port: %s" % UDP_SOCKET_PORT_SIM_REGISTRATION)

while True:
    data, addr = sock.recvfrom(1024)
    print("recevied message: %s" % data)