#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/carma/lib

cd /etc/ld.so.conf.d/

cat >> x86_64-linux-gnu.conf <<EOF
/usr/local/lib/
EOF
