## Build the custom V2X Hub image

1.  Download the V2X Hub source to your local machine.
2.  Download Docker and Docker compose based on the instructions available in Docker_Instructions.md
3.  Update the apt package index:
```
$ sudo apt-get update
```
4. Run mysql image using docker-compose. This may be done by creating a docker-compose.yml file with the following content. Replace the xyz.sql file with the database file created for the new build in the following code and place the file in the same directory as the docker-compose.yml file.
```
version: '3.7'

services:
  db:
    image: mysql
    container_name: mysql
    environment:
      - MYSQL_DATABASE=IVP
      - MYSQL_USER=IVP
      - MYSQL_PASSWORD=ivp
      - MYSQL_ROOT_PASSWORD=ivp
    network_mode: host
    volumes:
      - ./xyz.sql:/docker-entrypoint-initdb.d/xyz.sql
```
5.  To build the custom V2X Hub docker image, run the following command (change xyz to the name of the image):
```
$ sudo docker build --network=host -t xyz .
```
7.  To run the custom-built V2X Hub:
```
$ sudo docker run --network=host xyz
```
This will run the test build of V2X Hub in a docker contianer on your host computer while using mysql from the host computer.