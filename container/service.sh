#!/bin/bash
tmxctl --plugin CommandPlugin --enable
/usr/local/bin/tmxcore

# tmxctl --plugin CommandPlugin --enable
service tmxcore start
tail -f /var/log/tmx
