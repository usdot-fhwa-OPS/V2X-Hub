#!/bin/bash
/home/V2X-Hub/container/wait-for-it.sh 127.0.0.1:3306
chmod 777 /var/log/tmx
tmxctl --plugin CommandPlugin --enable
/usr/local/bin/tmxcore
