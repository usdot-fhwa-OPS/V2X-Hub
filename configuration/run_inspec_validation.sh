#!/bin/bash

# V2X-Hub InSpec Validation Runner
# This script runs the InSpec validation tests for V2X-Hub deployment

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

# Function to check if InSpec is installed
check_inspec() {
    if ! command -v inspec &> /dev/null; then
        print_status "ERROR" "InSpec is not installed"
        echo
        echo "Please install InSpec:"
        echo "  # Via gem:"
        echo "  gem install inspec"
        echo
        echo "  # Via package manager (Ubuntu/Debian):"
        echo "  wget https://packages.chef.io/files/stable/inspec/5.22.3/ubuntu/20.04/inspec_5.22.3-1_amd64.deb"
        echo "  sudo dpkg -i inspec_5.22.3-1_amd64.deb"
        exit 1
    fi
    print_status "SUCCESS" "InSpec is installed ($(inspec version))"
}

# Function to check if Docker Compose is accessible
check_docker_compose() {
    if ! docker compose ps >/dev/null 2>&1; then
        print_status "ERROR" "Docker Compose is not accessible or services are not running"
        echo
        echo "Please start V2X-Hub services:"
        echo "  docker compose up -d"
        exit 1
    fi
    print_status "SUCCESS" "Docker Compose services are accessible"
}

# Function to validate InSpec profile
validate_profile() {
    print_status "INFO" "Validating InSpec profile..."
    if inspec check inspec/v2xhub-validation >/dev/null 2>&1; then
        print_status "SUCCESS" "InSpec profile is valid"
    else
        print_status "ERROR" "InSpec profile validation failed"
        inspec check inspec/v2xhub-validation
        exit 1
    fi
}

# Function to run InSpec tests
run_tests() {
    local output_format=${1:-"cli"}
    local output_file=${2:-""}
    
    print_status "INFO" "Running V2X-Hub validation tests..."
    echo
    
    local cmd="inspec exec inspec/v2xhub-validation"
    
    case $output_format in
        "html")
            if [ -n "$output_file" ]; then
                cmd="$cmd --reporter html:$output_file"
                print_status "INFO" "Generating HTML report: $output_file"
            else
                cmd="$cmd --reporter html:v2xhub_validation_report.html"
                print_status "INFO" "Generating HTML report: v2xhub_validation_report.html"
            fi
            ;;
        "json")
            if [ -n "$output_file" ]; then
                cmd="$cmd --reporter json:$output_file"
                print_status "INFO" "Generating JSON report: $output_file"
            else
                cmd="$cmd --reporter json:v2xhub_validation_report.json"
                print_status "INFO" "Generating JSON report: v2xhub_validation_report.json"
            fi
            ;;
        "both")
            cmd="$cmd --reporter cli json:v2xhub_validation_report.json"
            print_status "INFO" "Generating CLI output and JSON report"
            ;;
        *)
            print_status "INFO" "Using CLI output format"
            ;;
    esac
    
    echo "Executing: $cmd"
    echo
    
    if eval $cmd; then
        echo
        print_status "SUCCESS" "V2X-Hub validation completed successfully!"
        
        if [[ "$output_format" == "html" ]]; then
            local report_file=${output_file:-"v2xhub_validation_report.html"}
            print_status "INFO" "Open the HTML report: file://$(pwd)/$report_file"
        elif [[ "$output_format" == "json" ]]; then
            local report_file=${output_file:-"v2xhub_validation_report.json"}
            print_status "INFO" "JSON report saved: $report_file"
        elif [[ "$output_format" == "both" ]]; then
            print_status "INFO" "JSON report saved: v2xhub_validation_report.json"
        fi
    else
        echo
        print_status "ERROR" "V2X-Hub validation failed!"
        print_status "INFO" "Check the output above for details on failed tests"
        exit 1
    fi
}

# Function to run specific control categories
run_specific_tests() {
    local category=$1
    
    case $category in
        "security")
            print_status "INFO" "Running security-focused tests..."
            inspec exec inspec/v2xhub-validation --controls=/database_spec.rb --controls=/ports_spec.rb --controls=/network_spec.rb
            ;;
        "network")
            print_status "INFO" "Running network connectivity tests..."
            inspec exec inspec/v2xhub-validation --controls=/network_spec.rb
            ;;
        "services")
            print_status "INFO" "Running service status tests..."
            inspec exec inspec/v2xhub-validation --controls=/services_spec.rb --controls=/health_spec.rb
            ;;
        "config")
            print_status "INFO" "Running configuration tests..."
            inspec exec inspec/v2xhub-validation --controls=/docker_compose_spec.rb --controls=/environment_spec.rb
            ;;
        *)
            print_status "ERROR" "Unknown test category: $category"
            echo "Available categories: security, network, services, config"
            exit 1
            ;;
    esac
}

# Function to display usage
usage() {
    echo "V2X-Hub InSpec Validation Runner"
    echo
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -f, --format FORMAT     Output format: cli, html, json, both (default: cli)"
    echo "  -o, --output FILE       Output file name (for html/json formats)"
    echo "  -c, --category CATEGORY Run specific test category: security, network, services, config"
    echo "  --check-only            Only validate the profile, don't run tests"
    echo
    echo "Examples:"
    echo "  $0                                    # Run all tests with CLI output"
    echo "  $0 -f html                           # Generate HTML report"
    echo "  $0 -f json -o my_report.json        # Generate custom JSON report"
    echo "  $0 -c security                       # Run only security tests"
    echo "  $0 --check-only                      # Validate profile only"
}

# Main execution
main() {
    local output_format="cli"
    local output_file=""
    local category=""
    local check_only=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -f|--format)
                output_format="$2"
                shift 2
                ;;
            -o|--output)
                output_file="$2"
                shift 2
                ;;
            -c|--category)
                category="$2"
                shift 2
                ;;
            --check-only)
                check_only=true
                shift
                ;;
            *)
                print_status "ERROR" "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
    done
    
    echo "V2X-Hub InSpec Validation Runner"
    echo "==============================="
    echo
    
    # Run checks
    check_inspec
    check_docker_compose
    validate_profile
    
    if [ "$check_only" = true ]; then
        print_status "SUCCESS" "Profile validation completed successfully!"
        exit 0
    fi
    
    echo
    
    # Run tests
    if [ -n "$category" ]; then
        run_specific_tests "$category"
    else
        run_tests "$output_format" "$output_file"
    fi
}

# Run main function with all arguments
main "$@"
