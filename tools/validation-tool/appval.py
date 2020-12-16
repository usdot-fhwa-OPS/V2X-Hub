#!/usr/bin/env python3

## The program will test validation for MAP, TIM, SPaT applications on different DUTs
## Arguments : 
##              [DUTIP] : ipaddres of the DUT 
##              [DUTPORTssh]: SSH port for DUT
##              []

import J2735 
import json
import sys 
import subprocess 


### Read in the json file and enable scp to transfer it 
### 

username="test"
dutip=""
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

