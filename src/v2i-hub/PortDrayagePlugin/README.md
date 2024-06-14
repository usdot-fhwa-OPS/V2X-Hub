# Port Drayage Plugin Documentation
## Introduction
The Port Drayage Plugin in V2x-Hub facilitates infrastructure, vehicle and container handling equipment (CHE) communication for port drayage operations. The plugin provides and montior drayage actions for a freight truck inside and between a mock port and staging area. The list of actions includes ENTER_STAGING_AREA, PICKUP, EXIT_STAGING_AREA, ENTER_PORT, DROPOFF, PORT_CHECKPOINT, HOLDING_AREA, and EXIT_PORT.

## Configuration/Deployment
Clone V2x-Hub GitHub repos:
```
git clone https://github.com/usdot-fhwa-OPS/V2X-Hub
```
Once downloaded, navigate to the configuration directory:
```
cd ~/V2X-Hub/configuration/
```
Create mysql_password and mysql_root_password:
```
mkdir secrets 
nano mysql_password.txt
nano mysql_root_password.txt
```

You may make the passwords whatever you like, but you will need to remember them:
```
Example: ivp
```

Launch V2x-Hub and Port Drayage web service
```
docker-compose up
```

Create a V2X Hub UI username and password. 
```
cd mysql
./add_v2xhub_user.bash
```

You may make these whatever you’d like, but will need to use them to log into the web UI. Example:
```
Username: v2xadmin
Password: V2xHub#321
```

You will then need to enter the mysql_password you created:
```
Example: ivp
```
Insert redefined set of actions into `PORT_DRAYAGE` database. Open terminal and run “mysql -uroot -pivp -h127.0.0.1”
```
use PORT_DRAYAGE
```

```
INSERT INTO `first_action` VALUES ('DOT-80550','CARGO_A',28.1249788,-81.8348897,'PICKUP','4bea1c45-e421-11eb-a8cc-000c29ae389d','32320c8a-e422-11eb-a8cc-000c29ae389d');
```

```
INSERT INTO `freight` VALUES ('DOT-80550',NULL,28.1232195,-81.8348278,'EXIT_STAGING_AREA','32320c8a-e422-11eb-a8cc-000c29ae389d','4ace39e6-ee36-11eb-9a03-0242ac130003'),('DOT-80550','CARGO_A',28.1249788,-81.8348897,'PICKUP','4bea1c45-e421-11eb-a8cc-000c29ae389d','32320c8a-e422-11eb-a8cc-000c29ae389d'),('DOT-80550',NULL,28.1128156,-81.8314745,'ENTER_PORT','4ace39e6-ee36-11eb-9a03-0242ac130003','67eadd3a-38b4-11ec-930a-000145098e4f'),('DOT-80550','CARGO_B',28.1232336,-81.8347566,'ENTER_STAGING_AREA','fc15d52a-3c0c-11ec-b00d-000145098e4f','5ceaab82-515c-11ec-9e2c-000145098e47');
```

```
INSERT INTO `freight` VALUES ('DOT-80550',NULL,28.1128156,-81.8314745,'ENTER_PORT','4ace39e6-ee36-11eb-9a03-0242ac130003','67eadd3a-38b4-11ec-930a-000145098e4f'),('DOT-80550','CARGO_A',28.1119763,-81.8312035,'DROPOFF','67eadd3a-38b4-11ec-930a-000145098e4f','0bf7ebda-38b5-11ec-930a-000145098e4f'),('DOT-80550','CARGO_B',28.1117373,-81.8309654,'PICKUP','0bf7ebda-38b5-11ec-930a-000145098e4f','9230504d-38b5-11ec-930a-000145098e4f'),('DOT-80550','CARGO_B',28.1120500,-81.8306483,'PORT_CHECKPOINT','9230504d-38b5-11ec-930a-000145098e4f','511ad052-38b6-11ec-930a-000145098e4f'),('DOT-80550','CARGO_B',28.1138052,-81.8317502,'EXIT_PORT','511ad052-38b6-11ec-930a-000145098e4f','fc15d52a-3c0c-11ec-b00d-000145098e4f'),('DOT-80550','CARGO_B',28.1232336,-81.8347566,'ENTER_STAGING_AREA','fc15d52a-3c0c-11ec-b00d-000145098e4f','5ceaab82-515c-11ec-9e2c-000145098e47');
```

Open a browser and get certificate in internet browser: Https://127.0.0.1:19760. 

Open another tab, navigate to  http://127.0.0.1 and land on V2x-Hub plugin management page. Enable `MessageReceiverPlugin` , `PortDrayagePlugin`, and `ImmediateForwardPlugin`.

## Functionality Testing or Regression Testing

Open another tab and type in `localhost:8090` to navigate to port drayage web UI. Click `Staging Area` or `Port Area` button to test V2x-Hub located at staging or port area.

Open a terminal, and run [a python script](https://raw.githubusercontent.com/usdot-fhwa-OPS/V2X-Hub/develop/configuration/mysql/suntrax/momscript_port_drayage.py) to send mocked mobility operation message to test each action at a particular location.
```
python3 momscript_port_drayage.py
```


