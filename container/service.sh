#!/bin/bash

tmxctl --plugin CommandPlugin --enable
service tmxcore start
tail -f /var/log/tmx
