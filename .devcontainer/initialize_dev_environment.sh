#!/bin/sh
/home/V2X-Hub/container/database.sh
/home/V2X-Hub/container/library.sh
ldconfig
# Install development tools 

apt update 
apt install -y valgrind gdb net-tools vim