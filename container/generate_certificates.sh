#!/bin/sh
set -e

mkdir -p /var/www/plugins/ssl 

# Check if certs already exist
if [ -f /var/www/plugins/ssl/cert.pem ] && [ -f /var/www/plugins/ssl/cert-key.pem ]; then
    echo "Certificates already exist, skipping generation"
    exit 0
else
    echo "Generating new self-signed certificates for V2X Hub IP $LOCAL_IP"
    openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout /var/www/plugins/ssl/cert-key.pem -out /var/www/plugins/ssl/cert.pem -config /home/V2X-Hub/container/san.cnf -days 365 -extensions v3_req
fi
chown plugin:adm -R /var/www/plugins/ssl