#!/bin/bash

cd /etc/ld.so.conf.d/

cat >> x86_64-linux-gnu.conf <<EOF
/usr/local/lib/
EOF
