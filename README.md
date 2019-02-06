Open Source Overview
============================
V2X Hub
Version 3.2
V2X Hub was developed to support jurisdictions in deploying connected vehicle technology by reducing 
integration issues and enabling use of their existing transportation management hardware and systems. 
V2X Hub is a software platform that utilizes plugins to translate messages between different devices 
and run transportation management and connected vehicle applications on roadside equipment.


Security and Passwords
----------------------
V2X Hub is middleware that runs on Linux Ubuntu 16.04 LTS. It is recommended that appropriate security 
and firewall settings be used on the computer running Linux, including conforming to your agency's 
security best practices and IT protocols.

For configuration and maintenance, the V2X Hub software includes an Administration Portal that runs 
in a web browser on the host device. The default Username for accessing this Administration Portal is 
v2iadmin and the default password is V2iHub#123. It is strongly recommended that the v2iadmin 
password be changed with the first login to the Administration Portal via the menu on the left. 
Passwords must be a minimum of 8 characters, with at least 1 number, 1 uppercase letter, 1 lowercase, 
and 1 special character.  Additional users can be created using the Administration Portal by selecting 
Manage Users from the left menu.  While managing users, you can add new users and delete the 
default user v2iadmin.  If you want to delete the v2iadmin default user, It is recommended that you 
create a new user with admin privileges, login as that user, then delete the v2iadmin default user.


License information
-------------------
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software distributed under
the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied. See the License for the specific language governing
permissions and limitations under the License.


System Requirements
-------------------
The V2X Hub software can run on most Linux based computers with 
Pentium core processers, with at least two gigabytes of RAM and at least 64GB of drive space.
Performance of the software will be based on the computing power and available RAM in 
the system.  The project was developed and tested on a machine 
with a core i3 processor, 4GB of memory, 64GB of hard drive space, running Ubuntu 16.04 LTS.

The V2X Hub software was developed using c and c++ and requires the following packages installed via apt-get:
cmake
gcc-5
g++-5
libboost1.58-dev
libboost-thread1.58-dev
libboost-regex1.58-dev
libboost-log1.58-dev
libboost-program
options1.58-dev
libboost1.58-all-dev
libxerces-c-dev
libcurl4-openssl-dev
libsnmp-dev
libmysqlclient-dev
libjsoncpp-dev
uuid-dev
git
libusb-dev
ibusb-1.0.0-dev
libftdi-dev
swig
liboctave-dev
gpsd libgps-dev
portaudio19-dev
libsndfile-dev
libev-dev
libuv-dev
libglib2.0-dev
libglibmm-2.4-dev
libpcre3-dev
libsigc++-2.0-dev
libxml++2.6-dev
libxml2-dev
liblzma-dev
dpkg-dev


Run the following command to install prerequisites via apt-get:
$ sudo apt-get install cmake gcc-7 g++-7 libboost1.65-dev libboost-thread1.65-dev libboost-regex1.65-dev libboost-log1.65-dev libboost-program-options1.65-dev libboost1.65-all-dev libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libjsoncpp-dev uuid-dev libusb-dev libusb-1.0-0-dev libftdi-dev swig liboctave-dev gpsd libgps-dev portaudio19-dev libsndfile1-dev libglib2.0-dev libglibmm-2.4-dev libpcre3-dev libsigc++-2.0-dev libxml++2.6-dev libxml2-dev liblzma-dev dpkg-dev libmysqlcppconn-dev libev-dev libuv-dev git

Compilation Instructions
------------------------


To Compile the V2X Hub software
Run the following from the src directory

$ cd tmx
$ cmake .
$ make 
$ sudo make install


The library path to the new libraries needs to added to the LD_LIBRARY_PATH variable for the libraries to be found. This can be done in a per session basis with the command line.

$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

When the unit is rebooted this variable will not be set. To add the path at bootup modify the correct file in the /etc/ld.so.conf.d/ directory and add a line with the path “/usr/local/lib/”. For an Intel 64 bit system the correct file is “x86_64-linux-gnu.conf” but this will vary based on your platform. After the file is modified run this command to update the paths.

$ sudo ldconfig

The V2X Hub supplied plugins have a dependency on a version of libwebsockets that is newer than the installable package that comes with Ubuntu 16.04.  Therefore, a custom version of the software needs to be downloaded and compiled locally before compiling the V2X Hub plugins.  Note this requires the GIT tool for checking out the latest version of the source code.

$ cd <some tmp dir>
$ git clone http://libwebsockets.org/repo/libwebsockets
$ cd libwebsockets
$ git checkout tags/v3.0.0
$ cmake -DLWS_WITH_SHARED=OFF .
$ make 
$ sudo make install

The new libwebsockets static library should now be available in /usr/local to build against.

Now, run the following from the v2i-hub directory
$ cmake .
$ make

This will create a bin directory that contains the plugin executable, as well as a directory for each plugin.  However, a V2X Hub plugin must be packaged in a ZIP file to be installed to a system.  In order to package up any one of the plugins from the v2i-hub directory, do the following:

$ ln -s ../bin <PluginName>/bin
$ zip <PluginName>.zip <PluginName>/bin/<PluginName> <PluginName>/manifest.json

The binary and the manifest file are the minimum number of files needed for any V2X Hub plugin.  It is possible some specific plugins require more files from the sub-directory to be included in the installable ZIP.


Installation Instructions
-------------------------
Installation Instructions
- install lamp-server
$ sudo apt-get install lamp-server^
	enter a root password (i.e. ivp)
- install database
  - modify the install_db.sh script located in the DatabaseSetup directory.  Modify the value for DBROOTPASS to the password that was used for root during the previous step
  - save the script
  - execute the script using the following commands
$ chmod +x install_db.sh
$ sudo ./install_db.sh
- To setup a service to start tmxcore on Ubuntu copy the tmxcore.service file located in the "src/tmx/TmxCore" directory to the “/lib/systemd/system/” directory. Execute the following commands to enable the application at startup.
$ sudo systemctl daemon-reload
$ sudo systemctl enable tmxcore.service
$ sudo systemctl start tmxcore.service


Set Up and Configuration Instructions

The CommandPlugin plugin must be running to access the Administration Portal. Follow the instructions above to build the CommandPlugin.zip package and then refer to Chapter 3 of the V2X Hub Administration Portal User Guide for installation and configuration instructions.

Instructions can be found to install additional plugins in the V2X Hub Software Configuration Guide.

Administration Portal

The Administrator Portal can be launched by opening the v2i-webportal/index.html file with either Chrome or Firefox. Further instructions for hosting the portal on a web server can be found in the V2X_Hub_AdministrationPortalUserGuide.pdf.


NOTE: The MAP plugin will need an input file in order to run.  A sample input file for Turner Fairbank has been included in this deployment in the Sample MAP Input folder.


- Copy sample MAP input file
$ sudo cp Sample MAP Input\ STOL_MAP.xml /var/www/plugins/MAPr41/
$ cd /var/www/plugins/MAP/
$ sudo chmod 644 STOL_MAP.xml
$ sudo chown www-data STOL_MAP.xml
$ sudo chgrp www-data STOL_MAP.xml
$ cd src

