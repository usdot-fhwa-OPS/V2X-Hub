# C1T Garage Actions
These are Port Drayage actions created for the Saxton garage for testing of C1T functionality.

## Instructions
Replace the port_drayage.sql file in docker-compose.yml with the file in this directory.
```
db:
   image: mysql:8.0
   volumes:
       - ./mysql/garage_port_drayage/port_drayage.sql:/docker-entrypoint-initdb.d/port_drayage.sql
```
