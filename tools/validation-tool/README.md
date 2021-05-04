
# Message Validation and Certification Tool v1.0
## A supplimental toolset for RSE Test Plans 

This tool/application is a part of USDOT's RoadSide Equipment testplans for RSE 4.0 standards. The application follows parallel to the methodology used to define how Intelligent Transportation Systems and Connected and Automated Vehicle message/applications are going to be validated in different ITS equipments. This tool is developed in python ver. 3 and available with docker build configurations for multi-platform support. This tool has been tested in Ubuntu 18.04.  Some of the usecases for this tool can be : 
* Certification testing of different applications 
* Researchers looking for quick message verification 

## Installation

The toolset can be installed either locally or through a docker container. Please use following instruction for installation based on the need. 

*Local Install*:

`sudo apt-get update -y && apt-get install -y python3-dev python3-pip git unzip python3-tk`

`git clone git@github.com:P1sec/pycrate.git`

`cd pycrate`

`sudo python3 setup.py install`

`cd ..`

`sudo pip3 install -r requirements.txt`

To run the application just enter: 

`python3 validationTool.py`

*Docker container*:

`sudo docker build -t <containerImageName> .`

`sudo apt-get install x11-xserver-utils && host +` <-- *(in order to allow docker access to x11 server)*

`sudo docker run --network=host -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:rw <containerImageName>`

## Usage

The application requires J2735.py library file needed for working with J2735 messages. It is advised that the user compile the SAE J2735 2016 asn schema using pycrate module to generate the J2735.py file. For licence issues the J2735.py file is not shared with the repository. 
The application requires a set of JSON files that contain the specific testing attrbitutes, these are basically test setup files. A snippet of the file for enabling MAP  testing is given below: 
```json
{ 
    "testgroup":"MAP",
    "testoperator": "person",
    "testdate": "",
    "testlist":
    {
        "Exist-01": 
        {   "testtype": "exist",
            "testobj" : "messageId"
        },
        "Exist-02": 
        {   "testtype": "exist",
            "testobj" : "msgIssueRevision"
        }
    }
}
```
As seen in the above JSON sample, each test has a name, e.g. "Exist-01" and has two attributes, "testtype"  and "testobj". The "testtype" defines whether its a test for certain field existance ("exist"), validation ("validate"), timing check ("timing") 
Some sample test setup files are available in the repository. These files need to be loaded in the application to setup a message validation testing. The layout of the application UI is shown below. The layout consists of 4 major sections as indicated in the figure. 

![Application UI](https://github.com/usdot-fhwa-OPS/V2X-Hub/blob/develop/tools/validation-tool/apppic1.png "Message Validation Application v1.0")

## Sample MAP test
The steps needed to run MAP test is given in figure below:
![MAP test](https://github.com/usdot-fhwa-OPS/V2X-Hub/blob/develop/tools/validation-tool/maptest.png "MAP test")

1. Input username and password for the DUTs SSH connection.
2. Input IP address and SSH port for connecting to the DUT.
3. Select whether a sample JSON message file is to be copied to a location on the DUT. The sample test message file should be copied for MAP and TIM test cases. 
4. If the message file is to be copied to the DUT, enter the location of the folder in DUT where the file is to be copied in the Remote filepath field. 
5. Select the local file that is to be copied to the device. This can be sample MAP, TIM files or sample SPAT custom file. This file will also be used in validation test cases. 
6. In the test setup block, input the test template file that defines which test categories are to be conducted. 
7. Input the port number for the TCP server that is used by the application for receiving captured packets. 
8. Input the bash script that can be run on the DUT for capturing the packets. These script is custom and can be edited based on need and device type. 
9. Hit the "Preview" button to view the sample message that is to be validated against as well as the set of check list that shows the different tests to be conducted. These new information are available in two new panes on the side. Any errors in the preview comes up in the debug pane. 
11. Check/Uncheck any tests that are not needed to be run in Test Information pane. 
12. Hit "Run" button and it does a couple of things:
    1. Initializes the TCP server to listen at the port specified above
    2. Copies and runs the bash script for capturing packets  
13. The application waits until the TCP server received a captured packet.
14. The result summary pane comes up when the test is conducted and results available. A report.csv file contains the different values for each attributes tested against is also created in the same location. 

## Sample SPAT test

The steps involved in conducting the SPAT test is illustrated in figure below:
![SPAT test](https://github.com/usdot-fhwa-OPS/V2X-Hub/blob/develop/tools/validation-tool/spattest.png "SPAT test")

1. Input username and password for the DUTs SSH connection.
2. Input IP address and SSH port for connecting to the DUT.
3. Select whether a sample JSON message file is to be copied to a location on the DUT. The sample test message file does not need to be copied for SPAT. 
4. Select the sample SPAT file that will used in validation test cases. 
5. In the test setup block, input the test template file that defines which test categories are to be conducted. 
7. Input the port number for the TCP server that is used by the application for receiving captured packets. 
8. Input the bash script that can be run on the DUT for capturing the packets. These script is custom and can be edited based on need and device type. 
9. Hit the "Preview" button to view the sample message that is to be validated against as well as the set of check list that shows the different tests to be conducted. These new information are available in two new panes on the side. Any errors in the preview comes up in the debug pane. 
11. Check/Uncheck any tests that are not needed to be run in Test Information pane. 
12. Hit "Run" button and it does a couple of things:
    1. Initializes the TCP server to listen at the port specified above
    2. Copies and runs the bash script for capturing packets  
13. The application waits until the TCP server received a captured packet.
14. The result summary pane comes up when the test is conducted and results available. A report.csv file contains the different values for each attributes tested against is also created in the same location. 



