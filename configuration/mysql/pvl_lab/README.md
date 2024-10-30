# PVL Actions
These are Port Drayage actions created for the PVL Lab for testing of CDA1tenth functionality.

## Instructions
Replace the port_drayage.sql file in docker-compose.yml with the file in this directory.
```
db:
   image: mysql:8.0
   volumes:
       - ./mysql/pvl_lab/port_drayage.sql:/docker-entrypoint-initdb.d/port_drayage.sql
```
