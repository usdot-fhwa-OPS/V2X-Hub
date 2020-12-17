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
import paramiko

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




def MAPtest(ip,port): 
    # copies MAP file to a designated location in the DUT 
    # captures the broadcast from the DUT wireless side 
    # print outs the decoded MAP 

    filename = loadfile()
    fp = open(filename,"r")

    ##  get username and password 
    #usernamessh = Guitextinput("username")
    #password = Guitextinput("password")

    uname = input("Enter Username for DUT login:  ")
    pwd = getpass("Enter Password for DUT login:  ")
    filepath = input("Enter the target file location:  ")
    print(uname, pwd)

    print("Sending MAP file to designated location")

    noIssue="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
    portconfig = "-P "+port
    process=subprocess.Popen(['scp',portconfig, noIssue, filename, uname+'@'+ip+':'+remotepath],stdout=subprocess.PIPE,stderr=subprocess.PIPE)

    client=paramiko.SSHClient()
    client.connect(ip,username=uname,password=pwd)
    stdin,stdout,stderr = cient.exec_command('ls')


    for line in stdout:
        print('.... '+ line.strip('\n'))
    client.close()







def main():

    print(len(sys.argv))

    if(len(sys.argv) != 4):
        print("Incomplete arguments")
        print("Usage:./appval.py [DUTIP] [DUTPORTssh] [TEST type] ")
        exit(0)



    username="test"
    dutip=sys.argv[1]
    dutportssh = sys.argv[2]
    testtype = sys.argv[3]

    print(dutip,dutportssh,testtype)

    if(testtype=="MAP"):
        MAPtest(dutip,dutportssh)

    password="test"
    remotepath=""
    filenamepath=""
    #noIssue="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null"
    #process=subprocess.Popen(['scp',filenamepath,username+'@'+dutip+':'+remotepath],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    regex='s/[ \t].*//;/^\(lo\|\)$/d'
    oifconfig = subprocess.Popen(['ifconfig','-a'],stdout = subprocess.PIPE)
    osed = subprocess.Popen(['sed',regex],stdin= oifconfig.stdout,stdout = subprocess.PIPE)
    endofline = osed.stdout

    #print("ifconfig -a |  sed 's/[ \t].*//;/^\(lo\|\)$/d'")

    for i in endofline:
        print('\n',i.strip())




if __name__ == "__main__":
    main()

    