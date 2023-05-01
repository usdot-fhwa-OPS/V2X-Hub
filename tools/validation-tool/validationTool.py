#!/usr/bin/env python3 


# the front end GUI 
# 
import sys
import json

from functools import partial
from getpass import getpass
from binascii import hexlify, unhexlify
import time
import socketserver

import socket
import binascii as bi 
from threading import Thread

# from paramiko import SSHClient
# from scp import SCPClient
import J2735 
from appJar import gui
from datetime import date
import collections
import pandas as pd 

report_file=open("report.csv","w")
createdPreview=False
createdTestSetup=False
createdTestResult=False
testlist="" 

def find(key, dictionary):
    for k, v in dictionary.items():
        if k == key:
            yield v
        elif isinstance(v, dict):
            for result in find(key, v):
                yield result
        elif isinstance(v, list):
            for d in v:
                if isinstance(d, dict):
                    for result in find(key, d):
                        yield result

def certify():
    global parsed, createdTestResult, testlist

    initialMessage = app.getTextArea("message content")

    # dict2str=json.dumps(parsed,indent=4,sort_keys=True,ensure_ascii=False)#.decode("utf-8","ignore")
    # parsed = json.loads(parsed,parse_int=str)



    initm = json.loads(initialMessage)
    dict2str2=json.dumps(initm,indent=4,sort_keys=True,ensure_ascii=False)
    initialMessage = json.loads(dict2str2,parse_int=str)#.decode("utf-8","ignore")
    #initialMessage = initm

    #json.dump(parsed,report_file)
    if(createdTestResult==False):
        app.startPanedFrame("r1")
        app.startLabelFrame("Results: Summary")
        testlist=testTemplate["testlist"]
        testtype=""
        testvalue=""
        for key,value in testlist.items():

            testvector=app.getCheckBox(key)

            if(testvector==True):
                app.addLabel(key)
                for testkey in  value: 
                    # print(testkey+ ":", value[testkey])
        
                    if(value[testkey]=="exist" or value[testkey]=="verify" or value[testkey] == "print"):
                        testtype=value[testkey]
                    else:
                        if(testtype=="exist"):
                            ret=list(find(value[testkey],parsed))
                            if(len(ret)>0):
                                app.setLabel(key,key+" :: "+value[testkey]+" :: PASSED")
                                app.setLabelFg(key,"green")
                            else:
                                app.setLabel(key,key+" :: "+value[testkey]+" :: FAILED")
                                app.setLabelFg(key,"red")
                        elif(testtype=="verify"):
                            msg1=list(find(value[testkey],parsed))
                            msg2=list(find(value[testkey],initialMessage))
                            msg1s= list(map(str, msg1)) 
                            msg2s= list(map(str,msg2)) 


                            report_file.write("%s:: (Received):: " %value[testkey])
                            for ls in msg1s:
                                report_file.write("%s , " %ls)
                            report_file.write("\n")

                            report_file.write("%s:: (Initial):: " %value[testkey])
                            for ls in msg2s:
                                report_file.write("%s , " %ls)
                            report_file.write("\n")
                            if(len(msg1s) >0 and len(msg2s)>0):
                                try:
                                    if(collections.Counter(msg1s)==collections.Counter(msg2s)):
                                        test = str(value[testkey])
                                        app.setLabel(key,key+" :: "+test+" :: PASSED")                                        
                                        app.setLabelFg(key,"green")
                                    else:
                                        app.setLabel(key,key+" :: "+value[testkey]+" :: FAILED")
                                        app.setLabelFg(key,"red")
                                except:
                                    app.setLabel(key,key+" :: "+value[testkey]+" :: ERROR")
                                    app.setLabelFg(key,"red")
                            else:
                                app.setLabel(key,key+" :: "+value[testkey]+" :: UNAVAILABLE")
                                app.setLabelFg(key,"orange")
                        elif(testtype=="print"):
                            ret=list(find(value[testkey],parsed))
                            if(len(ret)>0):
                                app.setLabel(key,key+" :: "+value[testkey]+":: "+str(ret))
                                app.setLabelFg(key,"green")
                            else:
                                app.setLabel(key,key+" :: "+value[testkey]+" :: UNAVAILABLE")
                                app.setLabelFg(key,"orange")                            

        app.stopLabelFrame()
        createdTestResult=True
    else:

        app.openPanedFrame("r1")
        app.emptyCurrentContainer()
        print("after remove and start")
        app.startLabelFrame("Results: Summary")
        testlist=testTemplate["testlist"]
        testtype=""
        testvalue=""
        for key,value in testlist.items():

            testvector=app.getCheckBox(key)

            if(testvector==True):
                app.addLabel(key)
                for testkey in  value: 
        
                    if(value[testkey]=="exist" or value[testkey]=="verify" or value[testkey] == "print"):
                        testtype=value[testkey]
                    else:
                        if(testtype=="exist"):
                            ret=list(find(value[testkey],parsed))
                            #print (value[testkey],ret)
                            if(len(ret)>0):
                                app.setLabel(key,key+" :: "+value[testkey]+" :: PASSED")
                                app.setLabelFg(key,"green")
                            else:
                                app.setLabel(key,key+" :: "+value[testkey]+" :: FAILED")
                                app.setLabelFg(key,"red")
                        elif(testtype=="verify"):
                            msg1=list(find(value[testkey],parsed))
                            msg2=list(find(value[testkey],initialMessage))
                            msg1s= list(map(str, msg1)) 
                            msg2s= list(map(str,msg2)) 

                            report_file.write("%s:: " %value[testkey])
                            for ls in msg2s:
                                report_file.write("%s , " %ls)
                            report_file.write("\n")

                            report_file.write("%s:: " %value[testkey])
                            for ls in msg1s:
                                report_file.write("%s , " %ls)
                            report_file.write("\n")
                            if(len(msg1s) >0 and len(msg2s)>0):
                                try:
                                    if(collections.Counter(msg1s)==collections.Counter(msg2s)):
                                        app.setLabel(key,key+" :: "+value[testkey]+" :: PASSED")                                        
                                        app.setLabelFg(key,"green")
                                    else:
                                        app.setLabel(key,key+" :: "+value[testkey]+" :: FAILED")
                                        app.setLabelFg(key,"red")
                                except:
                                    app.setLabel(key,key+" :: "+value[testkey]+" :: ERROR")
                                    app.setLabelFg(key,"red")
                            else:
                                app.setLabel(key,key+" :: "+value[testkey]+" :: UNAVAILABLE")
                                app.setLabelFg(key,"orange")
                        elif(testtype=="print"):
                            ret=list(find(value[testkey],parsed))
                            if(len(ret)>0):
                                app.setLabel(key,key+" :: "+value[testkey]+":: "+ret)
                                app.setLabelFg(key,"green")
                            else:
                                app.setLabel(key,key+" :: "+value[testkey]+" :: UNAVAILABLE")
                                app.setLabelFg(key,"orange") 
    
        app.stopLabelFrame()
        print("after redoing the results")

    app.stopAllPanedFrames()        

class UDPRecvHandle(socketserver.BaseRequestHandler):
    def handle(self):
        global parsed    
        self.data = self.request.recv(10000).strip()
        self.data = ''.join(self.data.split())

        self.msg = self.data.decode("utf-8","ignore")

        asnobj =  J2735.DSRC.MessageFrame
        asnobj.from_uper(unhexlify(self.msg))
        parsed = asnobj.to_json()  
        certify()
        


        

def runUDPserver(port):
    with socketserver.UDPServer(('', port), UDPRecvHandle) as server:
        print("Starting the TCP Server in localhost", "::", port)
        server.serve_forever()

# def runSSHSnifer(ssh, command):
#     print("sending ssh command")
#     stdin, stdout,stderr = ssh.exec_command(command)
#     print("done sending ssh comand")
#     for line in iter(stdout.readline, ""):
#         print(line, end="")


def run():
    ## this event initiates the test 
    initialMessage = app.getTextArea("message content")

    with open("./.localsample","w") as f:
        f.write(initialMessage)
    
    ## install test message to the proper location 

    # uname = app.getEntry("Username")
    # pword = app.getEntry("Password")
    # ipaddr = app.getEntry("Local IP address")
    # port = app.getEntry("Port #")
    # filepath = app.getEntry("Remote filepath")
    # filename = ".localsample"
    serverPort = int(app.getEntry("Server port"))
    # snifferScript = app.getTextArea("Sniffer Script")

    # print(uname, pword, ipaddr, port, filepath, filename,sport, snifscript)

    sk_listen = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sk_listen.bind(('127.0.0.1', serverPort))
    print("Starting the UDP Server in localhost", "::", serverPort)

    global parsed    
    data = str(sk_listen.recvfrom(10000)[0])
    data = ''.join(data.split())

    msgId = "0013"
    idx = data.find(msgId)
    if(int('0x'+data[idx+4],16)==8):
        if(msgId == '0014'):
            lenstr=int('0x'+data[idx+5:idx+8],16)*2+16
        else:
            lenstr=int('0x'+data[idx+5:idx+8],16)*2+8
    else:
        if(msgId == '0014'):
            lenstr=int('0x'+data[idx+5:idx+8],16)*2+16
        else:
            lenstr=int('0x'+data[idx+4:idx+6],16)*2+6
    msg = data[idx:idx+lenstr].encode('utf-8')

    asnobj =  J2735.DSRC.MessageFrame
    asnobj.from_uper(unhexlify(msg))
    parsed = asnobj.to_json()
    parsed = json.loads(parsed,parse_int=str)
    # print(type(parsed), parsed)
    certify()

    # ssh = SSHClient()
    # ssh.load_system_host_keys()

    # try:
    #     ssh.connect(hostname=ipaddr, 
    #             port = int(port),
    #             username=uname,
    #             password=pword)
    # except:
    #     print("Error ssh-ing to the DUT")
    #     ssh.close()
    #     return 0

    ## check if load file is yes 

    # if (app.getRadioButton("loadfile") == "yes"):
    #     # SCPCLient takes a paramiko transport as its only argument
    #     # scp = SCPClient(ssh.get_transport())
    #     # scp.put(filename, filepath)

    # try:
    #     t = Thread(target = runUDPserver, args=[serverPort],  daemon = True) 
    #     t.start()
    # except:
    #     print("Port already in use\n")

    # try:
    #     s = Thread(target = runSSHSnifer, args=(ssh,snifscript,),  daemon = True) 
    #     s.start()
    # except:
    #     print("Unable to send sniffer script to DUT")




def preview():
    global testTemplate, message, createdTestSetup
    app.clearLabel("er1")
    app.clearLabel("er2")
    app.clearLabel("er3")
    app.clearLabel("er4")    
    filepath1=app.getEntry("Local sample file")
    filepath2=app.getEntry("Test template")
    snifferscript=app.getTextArea("Sniffer Script")

    flag=1

    if( filepath1==""):
        app.setLabel("er1","Empty local sample file input")
        app.setLabelBg("er1","red")
        flag = 0 
    if( filepath2==""):
        app.setLabel("er2","Empty input test template")
        app.setLabelBg("er2","red")
        flag=0

    if(flag==0):
        return 0
    
    if( snifferscript==""):
        app.setLabel("er3","Empty sniffer script, Using default")
        app.setLabelBg("er3","red")
        app.setTextArea("Sniffer Script","sudo tcpdump -i lo port 1516 -Aq | grep -m  1 \"Payload=\" | sed 's|Payload=||g' | netcat <HOST IP> <HOST PORT>")
        
        
    

    

    file=open(filepath1,"r")
    sampleMsg=file.read()
    message=sampleMsg
    file.close()
    app.clearTextArea("message content")
    app.setTextArea("message content",message,end=False)

    print(filepath2)
    with open(filepath2) as file:
        testTemplate=json.load(file)
    
    setupTestTemplate()




def setupTestTemplate():
    global testTemplate, createdTestSetup,testlist

    #print(testTemplate)

    

    if(createdTestSetup==False):
        testlist=testTemplate["testlist"]
        app.startPanedFrame("t1")

        app.startLabelFrame("Test information")
        app.addLabel("Testgroup","Testgroup : "+testTemplate["testgroup"])
        app.addLabel("Testoperator","Operator : "+testTemplate["testoperator"])
        if(testTemplate["testdate"]==""):
            today = date.today()
            app.addLabel("Testdate","Date : "+today.strftime("%d/%m/%Y"))
        else:
            app.addLabel("Testdate","Date : "+testTemplate["testdate"])
    
        for keys in testlist.keys():
            app.addCheckBox(keys)
            app.setCheckBox(keys)
        createdTestSetup=True
        app.stopLabelFrame()
        app.addButton("Run", run)

    else:

        app.openLabelFrame("Test information")
        for keys in testlist.keys():
            app.removeCheckBox(keys)
        testlist=testTemplate["testlist"]
        app.setLabel("Testgroup","Testgroup : "+testTemplate["testgroup"])
        app.setLabel("Testoperator","Operator : "+testTemplate["testoperator"])
        if(testTemplate["testdate"]==""):
            today = date.today()
            app.setLabel("Testdate","Date : "+today.strftime("%d/%m/%Y"))
        else:
            app.setLabel("Testdate","Date : "+testTemplate["testdate"])
    
        for keys in testlist.keys():
            app.addCheckBox(keys)
            app.setCheckBox(keys)
        app.stopLabelFrame()


def loadfilechoice():
    if(app.getRadioButton("loadfile") == "Yes"):
        app.enableEntry("Remote filepath")
    else:
        app.disableEntry("Remote filepath")



app = gui()
message=""
testTemplate=""
parsed=""
initialMessage=""


#app=app1
app.setTitle("RSE Test Plan: Message Certification Tool v1.0")
app.setFont(size=11)
app.setSize("1800x800")
app.setGuiPadding(1, 1)
app.setLabelFont(size=11, underline=False)


#app.startFrame("m1")

app.startPanedFrame("ts1")
app.startLabelFrame("DUT Configurations")

# app.startLabelFrame("DUT ssh connection")
# app.setSticky("ew")
# app.addLabelEntry("Username")
# #app.setFocus("Username")
# app.setEntryDefault("Username","")
# app.addLabelSecretEntry("Password")
# app.stopLabelFrame()
# app.startLabelFrame("DUT networking")
# app.setSticky("ew")
# app.addLabelEntry("Local IP address")
# app.setEntryDefault("IP address", "127.0.0.1")
# app.addLabelNumericEntry("Port #")
# app.setEntryDefault("Port #", "1516")
# app.stopLabelFrame()
# app.startLabelFrame("DUT message file")
# app.setSticky("ew")
# app.addLabel("Load Sample Message to DUT")
# app.addRadioButton("loadfile","Yes",0,1,1,1)
# app.addRadioButton("loadfile","No",0,2,1,1)
# app.setRadioButtonChangeFunction("loadfile",loadfilechoice)
# app.addLabelEntry("Remote filepath")
# app.setEntryDefault("Remote filepath", "")
app.addLabelOpenEntry("Local sample file")
app.stopLabelFrame()
# app.stopLabelFrame()

app.startLabelFrame("Test Setup")
app.addLabelOpenEntry("Test template")
app.addLabelNumericEntry("Server port")
app.setEntryDefault("Server port", "1516")
app.stopLabelFrame()

#app.addLabel("snf1","Sniffer Script")
app.addTextArea("Sniffer Script")
app.setTextAreaAspect("Sniffer Script",400)


app.addButton("Preview",preview)

app.startLabelFrame("Debug")
app.addLabel("er1","")
app.addLabel("er2","")
app.addLabel("er3","")
app.addLabel("er4","")
app.stopLabelFrame()


app.startPanedFrame("ms1")
app.setSticky("news")
#app.addLabel("m1","Message Preview")
app.addScrolledTextArea("message content",1,1,2,2)


app.go()
