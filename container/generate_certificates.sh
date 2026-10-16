#!/bin/bash
set -e

mkdir -p /var/www/plugins/ssl 
# Strip trailing carriage returns (\r) and whitespaces
V2XHUB_IP=$(echo "$V2XHUB_IP" | tr -d '\r' | xargs)
if [[ "$V2XHUB_IP" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    export V2XHUB_IP_ENTRY="$V2XHUB_IP"
    export V2XHUB_DNS_ENTRY="localhost"  # Safe fallback for the DNS slot
else
    export V2XHUB_DNS_ENTRY="$V2XHUB_IP"
    export V2XHUB_IP_ENTRY="127.0.0.1"   # Safe fallback for the IP slot
fi
# Check if certs already exist
if [ -f /var/www/plugins/ssl/cert.pem ] && [ -f /var/www/plugins/ssl/cert-key.pem ]; then
    echo "Certificates already exist, skipping generation"
    exit 0
else
    echo "Generating new self-signed certificates for V2X Hub DNS $V2XHUB_DNS_ENTRY and IP $V2XHUB_IP_ENTRY"
    openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout /var/www/plugins/ssl/cert-key.pem -out /var/www/plugins/ssl/cert.pem -config /home/V2X-Hub/container/san.cnf -days 365 -extensions v3_req
fi
chown plugin:adm -R /var/www/plugins/ssl