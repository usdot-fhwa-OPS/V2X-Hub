
#!/bin/bash
set -e
# Check for ssl/cert-key.pem and ssl/cert.pem
if [[ ! -f "ssl/cert-key.pem" || ! -f "ssl/cert.pem" ]]; then
    echo "Incomplete or missing SSL certificates."
    # Clear certificates if only one exists
    if [[ -f "ssl/cert-key.pem" ]]; then
        echo "Only cert-key.pem found. Removing it..."
        rm -f ssl/cert-key.pem
    elif [[ -f "ssl/cert.pem" ]]; then
        echo "Only cert.pem found. Removing it..."
        rm -f ssl/cert.pem
    fi
    echo "Generating new certificates..."
    mkdir -p ssl
    cd ssl || exit
    # Check for mkcert command
    if command -v mkcert &>/dev/null; then
        echo "mkcert command found. Generating SSL certificates..."
    else
        echo "mkcert command not found. Installing mkcert..."
        if ! command -v curl &>/dev/null;  then
            sudo apt update
            sudo apt install -y curl
        fi
        sudo apt install libnss3-tools
        sudo curl -JLO "https://dl.filippo.io/mkcert/latest?for=linux/amd64"
        sudo chmod +x mkcert-v*-linux-amd64
        sudo cp mkcert-v*-linux-amd64 /usr/local/bin/mkcert
    fi
    mkcert -install
    mkcert localhost 127.0.0.1 ::1
    mv *-key.pem cert-key.pem
    mv localhost+* cert.pem
    sudo chmod ugo+r *.pem
    echo "SSL certificates generated successfully."
    cd .. || exit
fi
