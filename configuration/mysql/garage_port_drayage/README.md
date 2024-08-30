# C1T Garage Actions
These are Port Drayage actions created the CDA1Tenth functionality and specifically tailored for the Turner Fairbank Highway Research Center's Saxton Laboratory garage demonstration.

## Instructions
Replace the port_drayage.sql file in docker-compose.yml with the files in this directory.
```
db:
   image: mysql:8.0
   volumes:
       - ./mysql/garage_port_drayage/port_drayage.sql:/docker-entrypoint-initdb.d/port_drayage.sql
       - ./mysql/garage_port_drayage/port_drayage_lane1.sql:/docker-entrypoint-initdb.d/port_drayage_lane1.sql
       - ./mysql/garage_port_drayage/port_drayage_lane2.sql:/docker-entrypoint-initdb.d/port_drayage_lane2.sql
```
