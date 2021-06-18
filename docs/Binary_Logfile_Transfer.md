# Binary Logfile Transer
## Introduction
Both the SPaTLoggerPlugin and the MessageLoggerPlugin consume J2735 messages and create binary logfiles inside a volume inside `V2X-Hub/configuration/amd64/logs`. These files are intended for [ODE](https://github.com/usdot-jpo-ode/jpo-ode) and are not sent by the plugin but instead by the `V2X-Hub/configuration/amd64/logs/filewatchscript.sh`. The script uses rsync to transfer new files. Below is the script.

```
#!/bin/bash
ODELINK="ubuntu@ec2-18-234-192-123.compute-1.amazonaws.com:/home/ubuntu/ode/uploads/bsmlog"
echo $ODELINK
rsync -e 'ssh -i ./ODE.pem' -avhP --remove-source-files ./*.bin  $ODELINK
while inotifywait -r -e modify,create,move ./; do
   rsync -e 'ssh -i ./ODE.pem' -avhP --remove-source-files  ./*.bin  $ODELINK
done
```
## Process Explanation
Currently the SPaTLoggerPlugin upon startup will create json and binary log files with filenames based on the configuration value currently set. These files will be created in a volume which is located in `V2X-Hub/configuration/amd64/logs/` in your host machine and `var/log/tmx` in the V2X-Hub container. In addition to creating these files, the plugin will create a json directory and ode directory. Once the current binary file reaches the size specified in the configuration value both json and binary files will be renamed base on the date and time and moved into the json and ode directories respectively. In the ode directory the filewatchscript.sh with be running and will transfer the file to ODE and delete it on successful transfer.  

## Using filewatchscript.sh
1) First configure the `ODELINK` variable to reflect the host and directory where your [ODE](https://github.com/usdot-jpo-ode/jpo-ode) instance is running.
2) Move the script into the ode directory created by the SPaTLoggerPlugin or MessageLoggerPlugin
3) Also move a .pem file for the ec2 instance into this directory 
5) Finally run the script `./filewatchscript.sh` and the script should send files on everytime a binary logfile fills up and is moved into the ode directory.
