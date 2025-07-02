######## use this block to send message to V2X hub##################
####### The messages hit MessageReceiver Plugin first at port: 26789 ########
 
import socket
import binascii as ba
from time import sleep
 
udpip="127.0.0.1"
udpport=26789
sk = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)

    
# sample TCR
txt="00F4253A1C18DCCA072980C00800000000D0C173D4D781899B9E6DD1AC500011AC5121B0001121A0"

unhexed = ba.unhexlify(txt)
sk.sendto(unhexed,(udpip,udpport))

print('Sending: ', unhexed, '\n')

############## EOF#############################################