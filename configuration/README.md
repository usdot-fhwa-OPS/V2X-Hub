## Deployment Configuration
### Introduction
This directory contains deployment configurations for deploying V2X-Hub on both ARM64(arm64) and x86(amd64) architectures. 
### Deployment Instructions
Once downloaded, navigate to the directory corresponding to your computer’s processor (1) or (2):
```
cd ~/V2X-Hub/configuration/arm64/
cd ~/V2X-Hub/configuration/amd64/
```
Run the initialization script:
```
./initialization.sh
```
Follow the prompts during installation.

You will be prompted to create a mysql_password and mysql_root_password. You may make the passwords whatever you like, but you will need to remember them.
```
Example: ivp
```
You will also be prompted to create a V2X Hub username and password. You may make these whatever you’d like, but will need to use them to log into the web UI. Example:
```
Username: v2xadmin

Password: V2xHub#321
```
You will then need to enter the mysql_password you created in step 5a:
```
Example: ivp
```
After installation is complete, the script will automatically open a web browser with two tabs.

Navigate to the tab labeled as “Privacy Error” and select the “Advanced” button.

Click on “proceed to 127.0.0.1 (unsafe)”

Note: This page will not do anything when clicking proceed

Close the Privacy Error tab and wait for the initial V2X Hub tab to finish loading

Enter the login credentials you created in step 5b and login.

Installation complete!
