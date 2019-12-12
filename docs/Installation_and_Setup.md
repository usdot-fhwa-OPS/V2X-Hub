## Compilation Instructions

Download the dependencies based on the choice of Operating System:

Ubuntu 16.04: [Ubuntu 16.04 Requirements Document](Ubuntu_16.04_Requirements.md)

Ubuntu 18.04: [Ubuntu 18.04 Requirements Document](Ubuntu_18.04_Requirements.md)

To Compile the V2X Hub software, run the following commands from V2X-Hub directory.

```
$ cd src/tmx
$ cmake .
$ make 
$ sudo make install
```

The library path to the new libraries needs to added to the LD_LIBRARY_PATH variable for the libraries to be found. This can be done in a per session basis with the command line.

```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
```

When the unit is rebooted this variable will not be set. To add the path at bootup modify the correct file in the /etc/ld.so.conf.d/ directory and add a line with the path “/usr/local/lib/”. For an Intel 64 bit system the correct file is “x86_64-linux-gnu.conf” but this will vary based on your platform. After the file is modified run this command to update the paths.

```
$ sudo ldconfig
```

The V2X Hub supplied plugins have a dependency on a version of libwebsockets that is newer than the installable package that comes with Ubuntu. Hence a custom version of the software has benn forked and made available with V2X-Hub. Run the following commands from V2X-Hub directory.

```
$ cd ext/libwebsockets
$ cmake -DLWS_WITH_SHARED=OFF .
$ make
$ sudo make install
```

The new libwebsockets static library should now be available in /usr/local to build against.


An OPENAPI based Qt webservice is needed by the plugins for http requests processing. A custom generated code using OPENAPI framework is available through the V2X-Hub repo and is located in “ext/server" folder. To compile the web service as a shared library following steps are to be taken: 

```
$ cd ext/server
$ cmake .
$ make 
$ sudo make install 
```
Googletest is another utility used by the V2X-hub for unit tests. Here are the steps to install googletest. The googletest application has to be installed in the prerequisite step first before proceeding. 

```
$ cd /usr/src/googletest/googletest
$ sudo mkdir build
$ cd build
$ sudo cmake ..
$ sudo make
$ sudo cp libgtest* /usr/lib/
$ cd ..
$ sudo rm -rf build


$ sudo mkdir /usr/local/lib/googletest
$ sudo ln -s /usr/lib/libgtest.a /usr/local/lib/googletest/libgtest.a
$ sudo ln -s /usr/lib/libgtest_main.a /usr/local/lib/googletest/libgtest_main.a
```

Now, run the following commands from V2X-Hub directory.
```
$ cd src/v2i-hub
$ cmake . -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake
$ make
```

This will create a bin directory that contains the plugin executable, as well as a directory for each plugin.  However, a V2X Hub plugin must be packaged in a ZIP file to be installed to a system.  In order to package up any one of the plugins from the v2i-hub directory, do the following:

```
$ ln -s ../bin <PluginName>/bin
$ zip <PluginName>.zip <PluginName>/bin/<PluginName> <PluginName>/manifest.json
```

The binary and the manifest file are the minimum number of files needed for any V2X Hub plugin.  It is possible some specific plugins require more files from the sub-directory to be included in the installable ZIP.

## Installation Instructions
* Install lamp-server
```
$ sudo apt-get install lamp-server^
```
and enter a root password (i.e. ivp)
* Install database
	* modify the install_db.sh script located in the DatabaseSetup directory.  Modify the value for DBROOTPASS to the password that was used for root during the previous step
	* save the script
	* execute the script using the following commands
```
$ chmod +x install_db.sh
$ sudo ./install_db.sh
```
* To setup a service to start tmxcore on Ubuntu copy the tmxcore.service file located in the "src/tmx/TmxCore" directory to the “/lib/systemd/system/” directory. Execute the following commands to enable the application at startup.
```
$ sudo systemctl daemon-reload
$ sudo systemctl enable tmxcore.service
$ sudo systemctl start tmxcore.service
```

## Set Up and Configuration Instructions

The CommandPlugin plugin must be running to access the Administration Portal. Follow the instructions above to build the CommandPlugin.zip package and then refer to Chapter 3 of the V2X Hub [Administration Portal User Guide](https://usdot-carma.atlassian.net/wiki/spaces/CH/pages/174620673/V2X+Hub+Project+Documentation?preview=/174620673/174981124/V2X_Hub_AdministrationPortalUserGuide_3-2.docx) for installation and configuration instructions.

Instructions can be found to configure the other plugins in the [V2X Hub Software Configuration Guide](https://usdot-carma.atlassian.net/wiki/spaces/CH/pages/174620673/V2X+Hub+Project+Documentation?preview=/174620673/174817284/V2X_Hub_Software_Configuration_Guide_3-2.docx)

## Administration Portal

The Administrator Portal can be launched by opening the v2i-webportal/index.html (V2X-Hub/tools/v2i-webportal/) file with either Chrome or Firefox. Further instructions for hosting the portal on a web server can be found in the [Administration Portal User Guide](https://usdot-carma.atlassian.net/wiki/spaces/CH/pages/174620673/V2X+Hub+Project+Documentation?preview=/174620673/174981124/V2X_Hub_AdministrationPortalUserGuide_3-2.docx).

NOTE: The MAP plugin will need an input file in order to run.  A sample input file for Turner-Fairbank has been included in this deployment in the Sample MAP Input folder.

### Map Input File Location

You can paste the map input file to this location:
```
/var/www/plugins/MapPlugin/
```

## Security and Passwords
V2X Hub is middleware that runs on Linux Ubuntu 16.04 LTS or Ubuntu 18.04 LTS with future versions on Ubuntu 18.04 LTS. It is recommended that appropriate security and firewall settings be used on the computer running Linux, including conforming to your agency's security best practices and IT protocols.

For configuration and maintenance, the V2X Hub software includes an Administration Portal that runs in a web browser on the host device. The default Username for accessing this Administration Portal is v2iadmin and the default password is V2iHub#123. It is strongly recommended that the v2iadmin password be changed with the first login to the Administration Portal via the menu on the left. Passwords must be a minimum of 8 characters, with at least 1 number, 1 uppercase letter, 1 lowercase, and 1 special character.  Additional users can be created using the Administration Portal by selecting Manage Users from the left menu.  While managing users, you can add new users and delete the default user v2iadmin.  If you want to delete the v2iadmin default user, It is recommended that you create a new user with admin privileges, login as that user, then delete the v2iadmin default user.
