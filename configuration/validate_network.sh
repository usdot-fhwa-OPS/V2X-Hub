#!/bin/bash

# V2X-Hub Network Validation Script
# This script validates the network configuration and database connectivity
# for the V2X-Hub Docker deployment with network security enhancements

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "SUCCESS")
            echo -e "${GREEN}✓${NC} $message"
            ;;
        "ERROR")
            echo -e "${RED}✗${NC} $message"
            ;;
        "WARNING")
            echo -e "${YELLOW}⚠${NC} $message"
            ;;
        "INFO")
            echo -e "${BLUE}ℹ${NC} $message"
            ;;
    esac
}

# Function to check if Docker Compose is running
check_docker_compose() {
    print_status "INFO" "Checking Docker Compose status..."
    
    if ! docker compose ps >/dev/null 2>&1; then
        print_status "ERROR" "Docker Compose is not running or not accessible"
        echo "Please run 'docker compose up -d' first"
        exit 1
    fi
    
    print_status "SUCCESS" "Docker Compose is accessible"
}

# Function to check service status
check_service_status() {
    print_status "INFO" "Checking service status..."
    
    local services=$(docker compose ps --services --filter "status=running")
    
    for service in db php v2xhub; do
        if echo "$services" | grep -q "^$service$"; then
            print_status "SUCCESS" "Service '$service' is running"
        else
            print_status "ERROR" "Service '$service' is not running"
        fi
    done
}

# Function to check network connectivity
check_network_connectivity() {
    print_status "INFO" "Checking network connectivity..."
    
    # Check if v2xhub can reach database
    if docker compose exec -T v2xhub sh -c 'timeout 5 bash -c "</dev/tcp/db/3306"' >/dev/null 2>&1; then
        print_status "SUCCESS" "V2XHub can reach database (db)"
    else
        print_status "ERROR" "V2XHub cannot reach database"
    fi
    
    # Check if php can reach database
    if docker compose exec -T php sh -c 'timeout 5 bash -c "</dev/tcp/db/3306"' >/dev/null 2>&1; then
        print_status "SUCCESS" "PHP can reach database (db)"
    else
        print_status "ERROR" "PHP cannot reach database"
    fi
    
    # Check if v2xhub can reach php
    if docker compose exec -T php sh -c 'timeout 5 bash -c "</dev/tcp/v2xhub/19760"' >/dev/null 2>&1; then
        print_status "SUCCESS" "PHP can reach V2XHub service"
    else
        print_status "WARNING" "PHP cannot reach V2XHub service via websocket"
    fi

}

# Function to check WebSocket reverse proxy configuration
check_websocket_proxy() {
    print_status "INFO" "Checking WebSocket reverse proxy configuration..."

    # Check if Apache configuration file exists and contains WebSocket proxy settings
    if docker compose exec -T php test -f /etc/apache2/sites-available/000-default.conf >/dev/null 2>&1; then
        print_status "SUCCESS" "Apache configuration file exists"

        # Check for WebSocket proxy configuration
        if docker compose exec -T php grep -q "ProxyPass.*v2xhub-ws.*ws://" /etc/apache2/sites-available/000-default.conf 2>/dev/null; then
            print_status "SUCCESS" "WebSocket reverse proxy configuration found in Apache"
        else
            print_status "ERROR" "WebSocket reverse proxy configuration not found in Apache"
        fi

        # Check for proxy modules
        if docker compose exec -T php apache2ctl -M 2>/dev/null | grep -q "proxy_module\|proxy_wstunnel_module" >/dev/null 2>&1; then
            print_status "SUCCESS" "Apache proxy modules are loaded"
        else
            print_status "WARNING" "Apache proxy modules may not be loaded (check proxy_module and proxy_wstunnel_module)"
        fi
    else
        print_status "ERROR" "Apache configuration file not found"
    fi

    # Check if WebSocket environment variables are set in PHP container
    local ws_host=$(docker compose exec -T php printenv "V2XHUB_WS_HOST" 2>/dev/null || echo "")
    local ws_port=$(docker compose exec -T php printenv "V2XHUB_WS_PORT" 2>/dev/null || echo "")

    if [ -n "$ws_host" ]; then
        print_status "SUCCESS" "V2XHUB_WS_HOST is set to: $ws_host"
    else
        print_status "ERROR" "V2XHUB_WS_HOST environment variable not set in PHP container"
    fi

    if [ -n "$ws_port" ]; then
        print_status "SUCCESS" "V2XHUB_WS_PORT is set to: $ws_port"
    else
        print_status "ERROR" "V2XHUB_WS_PORT environment variable not set in PHP container"
    fi
}

# Function to check database connectivity
check_database_connectivity() {
    print_status "INFO" "Checking database connectivity..."
    
    # Wait for database to be ready
    local max_attempts=30
    local attempt=1
    
    while [ $attempt -le $max_attempts ]; do
        if docker compose exec -T db mysqladmin ping -h localhost --silent >/dev/null 2>&1; then
            print_status "SUCCESS" "Database is responding to ping"
            break
        else
            if [ $attempt -eq $max_attempts ]; then
                print_status "ERROR" "Database is not responding after $max_attempts attempts"
                return 1
            fi
            print_status "INFO" "Waiting for database... (attempt $attempt/$max_attempts)"
            sleep 2
            ((attempt++))
        fi
    done
    
    # TODO: Implement health check - Test database connection from v2xhub container
    # if docker compose exec -T v2xhub mysqladmin ping -h db -u IVP -p"$(docker compose exec -T v2xhub cat /run/secrets/mysql_password)" --silent >/dev/null 2>&1; then
    #     print_status "SUCCESS" "V2XHub can connect to database with credentials"
    # else
    #     print_status "ERROR" "V2XHub cannot connect to database with credentials"
    # fi
}

# Function to check environment variables
check_environment_variables() {
    print_status "INFO" "Checking environment variables..."
    
    local required_vars=("MYSQL_HOST" "MYSQL_PORT" "MYSQL_DATABASE" "MYSQL_USER")
    
    for var in "${required_vars[@]}"; do
        local value=$(docker compose exec -T v2xhub printenv "$var" 2>/dev/null || echo "")
        if [ -n "$value" ]; then
            print_status "SUCCESS" "$var is set to: $value"
        else
            print_status "ERROR" "$var is not set in v2xhub container"
        fi
    done
    
    # Check if password file exists
    if docker compose exec -T v2xhub test -f /run/secrets/mysql_password; then
        print_status "SUCCESS" "MySQL password file exists"
    else
        print_status "ERROR" "MySQL password file not found"
    fi
}

# Function to check network isolation
check_network_isolation() {
    print_status "INFO" "Checking network isolation..."
    
    # Check if database port is NOT exposed externally
    local db_external_port=$(docker compose port db 3306 2>/dev/null || echo "")
    if [ -z "$db_external_port" ] || [ "$db_external_port" = ":0" ]; then
        print_status "SUCCESS" "Database port 3306 is not exposed externally (secure)"
    else
        print_status "ERROR" "Database port 3306 is exposed externally: $db_external_port"
    fi

    local ws_external_port=$(docker compose port v2xhub 19760 2>/dev/null || echo "")
    if [ -z "$ws_external_port" ] || [ "$ws_external_port" = ":0" ]; then
        print_status "SUCCESS" "V2XHub WebSocket port 19760 is not exposed externally (secure)"
    else
        print_status "ERROR" "V2XHub WebSocket port 19760 is exposed externally: $ws_external_port"
    fi

    # Check internal network exists
    if docker network inspect configuration_v2xhub_internal >/dev/null 2>&1; then
        print_status "SUCCESS" "Internal network 'v2xhub_internal' exists"
        
        # Check if internal network is actually internal
        local internal=$(docker network inspect configuration_v2xhub_internal --format '{{.Internal}}' 2>/dev/null || echo "false")
        if [ "$internal" = "true" ]; then
            print_status "SUCCESS" "Internal network is properly configured (no external access)"
        else
            print_status "WARNING" "Internal network allows external access"
        fi
    else
        print_status "ERROR" "Internal network 'v2xhub_internal' not found"
    fi
    
    # Check external network exists
    if docker network inspect configuration_v2xhub_external >/dev/null 2>&1; then
        print_status "SUCCESS" "External network 'v2xhub_external' exists"
    else
        print_status "ERROR" "External network 'v2xhub_external' not found"
    fi
}

# Function to check exposed ports
check_exposed_ports() {
    print_status "INFO" "Checking exposed ports..."
    
    # Core ports that should always be exposed - check via Docker port mapping
    local core_services_ports=("php:80" "php:443" "v2xhub:8686")
    
    for service_port in "${core_services_ports[@]}"; do
        local service=$(echo "$service_port" | cut -d: -f1)
        local port=$(echo "$service_port" | cut -d: -f2)
        
        # Check if Docker has mapped this port
        local mapped_port=$(docker compose port "$service" "$port" 2>/dev/null || echo "")
        if [ -n "$mapped_port" ] && [ "$mapped_port" != ":0" ]; then
            print_status "SUCCESS" "Core port $port is exposed via $service service ($mapped_port)"
        else
            # Fallback to netstat check for host-bound ports
            if netstat -tuln 2>/dev/null | grep -q ":$port "; then
                print_status "SUCCESS" "Core port $port is exposed (host-bound)"
            else
                print_status "WARNING" "Core port $port is not exposed (check if $service service is running)"
            fi
        fi
    done
    
    # Check that WebSocket port 19760 is NOT exposed externally (should be internal only)
    local ws_mapped_port=$(docker compose port v2xhub 19760 2>/dev/null || echo "")
    if [ -n "$ws_mapped_port" ] && [ "$ws_mapped_port" != ":0" ]; then
        print_status "ERROR" "WebSocket port 19760 is exposed externally via Docker ($ws_mapped_port) - security risk"
    elif netstat -tuln 2>/dev/null | grep -q ":19760 "; then
        print_status "ERROR" "WebSocket port 19760 is exposed externally (security risk - should be internal only)"
    else
        print_status "SUCCESS" "WebSocket port 19760 is not exposed externally (secure - handled via reverse proxy)"
    fi

    # Check for profile-specific ports using both Docker port mapping and netstat
    local profile_ports=("26789" "22222" "44444" "6767" "7575" "5757" "7576" "8090")
    local exposed_profile_ports=()
    
    for port in "${profile_ports[@]}"; do
        # Check if any service has this port mapped
        local port_found=false
        for service in $(docker compose ps --services 2>/dev/null || echo ""); do
            local mapped=$(docker compose port "$service" "$port" 2>/dev/null || echo "")
            if [ -n "$mapped" ] && [ "$mapped" != ":0" ]; then
                exposed_profile_ports+=("$port")
                port_found=true
                break
            fi
        done

        # Fallback to netstat if not found via Docker
        if [ "$port_found" = false ] && netstat -tuln 2>/dev/null | grep -q ":$port "; then
            exposed_profile_ports+=("$port")
        fi
    done
    
    if [ ${#exposed_profile_ports[@]} -gt 0 ]; then
        print_status "INFO" "Profile-specific ports exposed: ${exposed_profile_ports[*]}"
        print_status "WARNING" "Ensure these ports are needed for your deployment"
    else
        print_status "SUCCESS" "No profile-specific ports exposed (minimal attack surface)"
    fi
}

# Function to check health status
check_health_status() {
    print_status "INFO" "Checking container health status..."
    
    local services=("v2xhub" "v2xhub_with_simulation")
    
    for service in "${services[@]}"; do
        local health=$(docker compose ps --format "table {{.Service}}\t{{.Status}}" | grep "^$service" | awk '{print $2}' || echo "")
        if [ -n "$health" ]; then
            if echo "$health" | grep -q "healthy"; then
                print_status "SUCCESS" "Service '$service' is healthy"
            elif echo "$health" | grep -q "unhealthy"; then
                print_status "ERROR" "Service '$service' is unhealthy"
            else
                print_status "INFO" "Service '$service' health status: $health"
            fi
        fi
    done
}

# Function to display summary
display_summary() {
    echo
    print_status "INFO" "=== Network Validation Summary ==="
    echo
    print_status "INFO" "Network Architecture:"
    echo "  • Internal Network: v2xhub_internal (172.20.0.0/16) - Database communication"
    echo "  • External Network: v2xhub_external (172.21.0.0/16) - External access"
    echo
    print_status "INFO" "Database Configuration:"
    echo "  • Host: db (Docker service name)"
    echo "  • Port: 3306 (internal only)"
    echo "  • Database: IVP"
    echo "  • User: IVP"
    echo "  • Password: Managed via Docker secrets"
    echo
    print_status "INFO" "WebSocket Configuration:"
    echo "  • WebSocket service: v2xhub:19760 (internal only)"
    echo "  • External access: Apache reverse proxy (/v2xhub-ws)"
    echo "  • No direct external WebSocket port exposure"
    echo
    print_status "INFO" "Security Features:"
    echo "  • Database isolated on internal network"
    echo "  • No direct external database access"
    echo "  • WebSocket connections via secure reverse proxy"
    echo "  • Service-to-service communication via Docker DNS"
    echo "  • Profile-based port exposure"
}

# Main execution
main() {
    echo "V2X-Hub Network Validation Script"
    echo "================================="
    echo
    
    check_docker_compose
    check_service_status
    check_network_connectivity
    check_websocket_proxy
    check_database_connectivity
    check_environment_variables
    check_network_isolation
    check_exposed_ports
    check_health_status
    display_summary
    
    echo
    print_status "SUCCESS" "Network validation completed!"
    echo
    print_status "INFO" "If you found any issues, please check:"
    echo "  • Docker Compose services are running: docker compose ps"
    echo "  • Container logs: docker compose logs <service_name>"
    echo "  • Network configuration: docker network ls"
    echo "  • Environment variables: docker compose config"
}

# Run main function
main "$@"
