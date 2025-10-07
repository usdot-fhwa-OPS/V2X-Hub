#!/bin/sh
set -ex
# These directories should already exist due to docker volumes
mkdir -p /var/www/plugins/MAP /var/www/plugins/ssl
# Check if certs already exist
if [ -f /var/www/plugins/ssl/cert.pem ] && [ -f /var/www/plugins/ssl/key.pem ]; then
    echo "Certificates already exist, skipping generation"
    exit 0
else
    echo "Generating new self-signed certificates for V2X Hub IP $V2XHUB_IP"
    openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout /var/www/plugins/ssl/cert-key.pem -out /var/www/plugins/ssl/cert.pem -subj "/CN=$V2XHUB_IP" -days 3650
    chown -R plugin:www-data /var/www/plugins/ssl
fi
