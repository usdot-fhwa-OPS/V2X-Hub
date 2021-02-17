
# Message Validation and Certification Tool v1.0
## A supplimental toolset for RSE Test Plans 

This tool/application is a part of USDOT's RoadSide Equipment testplans for RSE 4.0 standards. The application follows parallel to the methodology used to define how Intelligent Transportation Systems and Connected and Automated Vehicle message/applications are going to be validated in different ITS equipments. This tool is developed in python ver. 3 and available with docker build configurations for multi-platform support. This tool has been tested in Ubuntu 18.04.  Some of the usecases for this tool can be : 
* Certification testing of different applications 
* Researchers looking for quick message verification 

## Installation

The toolset can be installed either locally or through a docker container. Please use following instruction for installation based on the need. 

*Local Install*:

`sudo apt-get update -y && apt-get install -y python3-dev python3-pip git unzip python3-tk`

`unzip pycrate-master.zip`

`cd pycrate-master`

`python3 setup.py install`

`pip3 install -r requirements.txt`

To run the application just enter: 

`python3 validationTool.py`

*Docker container*:

`sudo docker build -t <containerImageName> .`

`sudo apt-get install x11-xserver-utils && host +` <-- *(in order to allow docker access to x11 server)*

`sudo docker run --network=host -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:rw <containerImageName>`

## Usage

The application requires a set of JSON files that contain the specific testing attrbitutes, these are basically a test setup files. A snippet of the file for enabling MAP  testing is given below: 
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
Some sample test setup files are available in the repository. These files need to be loaded in the application to setup a certification testing. 




