#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Load .env if present to get V2XHUB_VOLUME_PATH
if [ -f "${SCRIPT_DIR}/.env" ]; then
  source "${SCRIPT_DIR}/.env"
fi
# Ensure required environment variable is set
: "${V2XHUB_VOLUME_PATH:?V2XHUB_VOLUME_PATH must be set}"

SSL_DIR="${V2XHUB_VOLUME_PATH}/ssl"
# Check for ssl/cert-key.pem and ssl/cert.pem
if [[ ! -f "$SSL_DIR/cert-key.pem" || ! -f "$SSL_DIR/cert.pem" ]]; then
    echo "Incomplete or missing SSL certificates."
    # Clear certificates if only one exists
    if [[ -f "$SSL_DIR/cert-key.pem" ]]; then
        echo "Only cert-key.pem found. Removing it..."
        rm -f "$SSL_DIR/cert-key.pem"
    elif [[ -f "$SSL_DIR/cert.pem" ]]; then
        echo "Only cert.pem found. Removing it..."
        rm -f "$SSL_DIR/cert.pem"
    fi
    echo "Generating new certificates..."
    mkdir -p "$SSL_DIR"
    
    # Detect system architecture
    ARCH=$(uname -m)
    case ${ARCH} in
        x86_64)
            MKCERT_ARCH="amd64"
            ;;
        aarch64)
            MKCERT_ARCH="arm64"
            ;;
        *)
            echo "Unsupported architecture: ${ARCH}"
            exit 1
            ;;
    esac
    echo "Detected architecture: ${ARCH} (mkcert for linux/${MKCERT_ARCH})"
    
    # Check if mkcert exists
    MKCERT_NEEDS_INSTALL=false
    if command -v mkcert &>/dev/null; then
        echo "mkcert command found. Verifying functionality..."
        if ! mkcert -version &>/dev/null; then
            echo "mkcert exists but cannot execute. Reinstalling..."
            sudo rm -f /usr/local/bin/mkcert
            MKCERT_NEEDS_INSTALL=true
        else
            echo "mkcert is working correctly."
        fi
    else
        echo "mkcert command not found. Installing mkcert..."
        MKCERT_NEEDS_INSTALL=true
    fi
    
    # Install mkcert if needed
    if [ "$MKCERT_NEEDS_INSTALL" = true ]; then
        if ! command -v curl &>/dev/null; then
            sudo apt update
            sudo apt install -y curl
        fi
        sudo apt install -y libnss3-tools
        sudo curl -JLO "https://dl.filippo.io/mkcert/latest?for=linux/${MKCERT_ARCH}"
        sudo chmod +x mkcert-v*-linux-${MKCERT_ARCH}
        sudo cp mkcert-v*-linux-${MKCERT_ARCH} /usr/local/bin/mkcert
        # Delete download after install
        sudo rm mkcert-v*-linux-${MKCERT_ARCH}
        echo "mkcert installed successfully for ${MKCERT_ARCH}."
    fi
    mkcert -install
    mkcert -cert-file "$SSL_DIR/cert.pem" -key-file "$SSL_DIR/cert-key.pem" localhost 127.0.0.1 ::1
    sudo chmod ugo+r "$SSL_DIR/cert.pem" "$SSL_DIR/cert-key.pem""
    echo "SSL certificates generated successfully."
fi
