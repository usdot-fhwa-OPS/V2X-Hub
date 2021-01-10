#!/usr/bin/env python3

#######################################################################################
## The program will test validation for MAP, TIM, SPaT applications on different DUTs
## Arguments : ./appval.py [DUTIP] [DUTPORTssh] [TEST type]
##              [DUTIP] : ipaddres of the DUT 
##              [DUTPORTssh]: SSH port for DUT
##              [TEST type]: MAP/TIM/SPaT
################ Author: Anjan Rayamajhi  #############################################
#######################################################################################

import J2735 
import json
import sys 
import subprocess 
import tkinter as tk
import argparse as arg
from tkinter.filedialog import askopenfilename
from functools import partial
from getpass import getpass
from binascii import hexlify, unhexlify
import time

import socket
import binascii as bi 

from paramiko import SSHClient
from scp import SCPClient

### Read in the json file and enable scp to transfer it 
### 



def loadfile():
    tk.Tk().withdraw() # we don't want a full GUI, so keep the root window from appearing
    filename = askopenfilename() # show an "Open" dialog box and return the path to the selected file
    print(filename)
    return filename

def closewindow(window):
    window.destroy()
	 

def Guitextinput(label):
    tkWindow = tk.Tk()
    tkWindow.geometry('400x150')  
    tkWindow.title('Input Required')
    textlabel = tk.Label(tkWindow, text=label).grid(row=0, column=0)
    textinput = tk.StringVar()
    if(label=="password"):
        textEntry = tk.Entry(tkWindow, textvariable=textinput,show='*').grid(row=0, column=1) 
    else:     
        textEntry = tk.Entry(tkWindow, textvariable=textinput).grid(row=0, column=1)  
    closewindow1 = partial(closewindow,tkWindow)
    okbutton = tk.Button(tkWindow, text="Ok", command=closewindow1).grid(row=2, column=1)  
    tkWindow.mainloop()
    
    return textinput.get()




def SRME(ip,port): 
    # copies a file to a designated location in the DUT 
    # captures the broadcast from the DUT wireless side 
    # print outs the decoded message 

    print("Undergoing Store and Repeat With Encoding testing")
    print("Assumptions: The DUT has SRME enabled")
    print("Sending the JSON file to specific folder in the DUT")
    
    filename = loadfile()
    fp = open(filename,"r")

    ##  get username and password 
    #usernamessh = Guitextinput("username")
    #password = Guitextinput("password")

    uname = input("Enter Username for DUT login:  ")
    pwd = getpass("Enter Password for DUT login:  ")
    filepath = input("Enter target location:  ")
    print(uname, pwd)

    print("Sending MAP file to designated location")

    noIssue="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
    portconfig = "-P "+port
    process=subprocess.Popen(['scp',portconfig, noIssue, filename, uname+'@'+ip+':'+filepath],stdout=subprocess.PIPE,stderr=subprocess.PIPE)



    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(hostname=ip, 
                port = 22,
                username=uname,
                password=pwd)


    # SCPCLient takes a paramiko transport as its only argument
    scp = SCPClient(ssh.get_transport())

    scp.put(filename, filepath)
    #scp.get('file_path_on_remote_machine', 'file_path_on_local_machine')


    scp.close()
    ssh.close()



def SRM(ip,port):
    print("Reading from a sample file")
    fp = open("tempSample",'r')
    jMap = fp.read()

    mapasnobj =  J2735.DSRC.MessageFrame
    mapasnobj.from_uper(unhexlify(jMap))


    #print(mapasnobj())
    t=mapasnobj()

    print("t['value'][1]['msgIssueRevision'] = ",t['value'][1]['msgIssueRevision'])
    t['value'][1]['msgIssueRevision'] = 5

    mapasnobj.set_val(t)

    print("t['value'][1]['msgIssueRevision'] = ",t['value'][1]['msgIssueRevision'])


    print(hexlify(mapasnobj.to_uper()))
    uperhex=hexlify(mapasnobj.to_uper())

    print("SNMP setup")

    uname = input("Enter Username for DUT login:  ")
    pwd = getpass("Enter Password for DUT login:  ")

    ssh = SSHClient()
    ssh.load_system_host_keys()
    ssh.connect(hostname=ip, 
                port = 22,
                username=uname,
                password=pwd)

    #stdin,stdout,stderr=ssh.exec_command('ls')

    stdin,stdout,stderr=ssh.exec_command('/opt/cohda/application/rc.local stop',get_pty=True)
    time.sleep(3)
    for line in iter(stdout.readline, ""):
        print(line, end="")
    
    stdin,stdout,stderr=ssh.exec_command('net-snmp-config --create-snmpv3-user -A password -X password -a SHA -x AES leidos')
    time.sleep(3)
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('/opt/cohda/application/rc.local start')   
    time.sleep(3)    
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 iso.0.15628.4.1.99.0 i 2')
    time.sleep(3) 
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.11.1 i 5')
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.2.1 x 0x8002')
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.3.1 i 18')
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.4.1 i 0')
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.5.1 i 172')
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.6.1 i 1000')
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.9.1 x '+uperhex) 
    for line in iter(stdout.readline, ""):
        print(line, end="")
    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 1.0.15628.4.1.4.1.10.1 i 1')
    for line in iter(stdout.readline, ""):
        print(line, end="")

    stdin,stdout,stderr=ssh.exec_command('snmpset -v3 -lauthPriv -uleidos -Apassword -Xpassword -aSHA -xAES -mRSU-MIB -M/mnt/rw/rsu1609/snmp/mibs -O T 127.0.0.1 iso.0.15628.4.1.99.0 i 4')
    for line in iter(stdout.readline, ""):
        print(line, end="")



    ssh.close()



def sendTSCBM():
    sk = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    dutip = input("Enter DUT ip address:  ")
    dutport = input("Enter DUT port:  ")




def main():

    print(len(sys.argv))

    if(len(sys.argv) != 4):
        print("Incomplete arguments")
        print("Usage:./appval.py [DUTIP] [DUTPORTssh] [TEST type:SRM|SRME|SPATEncode] ")
        exit(0)



    dutip=sys.argv[1]
    dutportssh = sys.argv[2]
    testtype = sys.argv[3]

    print(dutip,dutportssh,testtype)

    if(testtype=="SRME"): # store and repeat with encoding 
        SRME(dutip,dutportssh)


    if(testtype=="SRM"): #store and repeat 
        SRM(dutip,dutportssh)


    if(testtype=="SPATEncode"):
        sendTSCBM()

if __name__ == "__main__":
    main()

    