#!/bin/bash
#
# PCIe Test Driver - Test Script
#
# This script provides automated testing of the PCIe test driver
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_NAME="pcie_test_driver"
MODULE_PATH="${SCRIPT_DIR}/${MODULE_NAME}.ko"
UTIL_PATH="${SCRIPT_DIR}/pcie_test_utility"
TEST_LOG="${SCRIPT_DIR}/test_results.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $*"
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] [INFO] $*" >> "${TEST_LOG}"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] [WARN] $*" >> "${TEST_LOG}"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $*"
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] [ERROR] $*" >> "${TEST_LOG}"
}

# Checkpoint tracking
declare -A CHECKPOINTS
CHECKPOINT_FILE="${SCRIPT_DIR}/.checkpoints"

checkpoint_set() {
    local name="$1"
    local status="$2"  # passed, failed, skipped
    local timestamp=$(date +'%Y-%m-%d %H:%M:%S')
    echo "[${timestamp}] CHECKPOINT: $name = $status" >> "${TEST_LOG}"
    CHECKPOINTS["$name"]="$status"
    echo -e "${GREEN}✓ CHECKPOINT${NC} [$name] = $status"
}

checkpoint_list() {
    echo ""
    echo -e "${GREEN}Checkpoint Summary:${NC}"
    echo "===================="
    for checkpoint in "${!CHECKPOINTS[@]}"; do
        local status="${CHECKPOINTS[$checkpoint]}"
        case $status in
            passed)  echo -e "${GREEN}✓${NC} $checkpoint" ;;
            failed)  echo -e "${RED}✗${NC} $checkpoint" ;;
            skipped) echo -e "${YELLOW}⊘${NC} $checkpoint" ;;
        esac
    done
    echo ""
}

# Usage
usage() {
    cat << EOF
Usage: $0 [options] <action>

Actions:
  build           Build the driver and test utility
  load            Load the driver module
  unload          Unload the driver module
  reload          Reload the driver module
  test-basic      Run basic functionality tests
  test-all        Run all tests
  check           Check driver status
  checkpoints     Show checkpoint report
  cleanup         Clean build artifacts

Options:
  -v              Verbose output
  -h              Show this help message
EOF
}

# Parse arguments
VERBOSE=0
while getopts "vh" opt; do
    case $opt in
        v) VERBOSE=1 ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done

shift $((OPTIND-1))
ACTION="${1:-help}"

# Ensure we run as root for module operations
check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_error "This script must be run as root for module operations"
        exit 1
    fi
}

# Build the driver and utility
build() {
    log_info "Building PCIe test driver..."
    cd "${SCRIPT_DIR}"
    
    if ! make clean > /dev/null 2>&1; then
        log_warn "Clean failed (may not exist)"
        checkpoint_set "clean_build" "skipped"
    else
        checkpoint_set "clean_build" "passed"
    fi
    
    if ! make modules > /dev/null 2>&1; then
        log_error "Build failed"
        checkpoint_set "compile_module" "failed"
        make modules
        return 1
    fi
    checkpoint_set "compile_module" "passed"
    
    log_info "Building test utility..."
    if ! gcc -o "${UTIL_PATH}" pcie_test_utility.c -Wall -Wextra 2>&1; then
        log_error "Utility build failed"
        checkpoint_set "compile_utility" "failed"
        return 1
    fi
    checkpoint_set "compile_utility" "passed"
    
    chmod +x "${UTIL_PATH}"
    checkpoint_set "executable_permissions" "passed"
    
    log_info "Build successful"
    log_info "  Module: ${MODULE_PATH}"
    log_info "  Utility: ${UTIL_PATH}"
    checkpoint_list
}

# Load module
load_module() {
    check_root
    
    if lsmod | grep -q "${MODULE_NAME}"; then
        log_warn "Module already loaded"
        checkpoint_set "module_loaded" "skipped"
        return 0
    fi
    
    if [[ ! -f "${MODULE_PATH}" ]]; then
        log_error "Module not found: ${MODULE_PATH}"
        log_info "Run: $0 build"
        checkpoint_set "module_exists" "failed"
        return 1
    fi
    checkpoint_set "module_exists" "passed"
    
    log_info "Loading module..."
    if insmod "${MODULE_PATH}"; then
        checkpoint_set "insmod_success" "passed"
        sleep 1
        
        if lsmod | grep -q "${MODULE_NAME}"; then
            checkpoint_set "module_loaded" "passed"
            log_info "Module loaded successfully"
            return 0
        else
            checkpoint_set "module_loaded" "failed"
            log_error "Module loaded but not found in lsmod"
            return 1
        fi
    else
        checkpoint_set "insmod_success" "failed"
        log_error "Failed to load module"
        return 1
    fi
}

# Unload module
unload_module() {
    check_root
    
    if ! lsmod | grep -q "${MODULE_NAME}"; then
        log_warn "Module not loaded"
        checkpoint_set "module_unloaded" "skipped"
        return 0
    fi
    
    log_info "Unloading module..."
    if rmmod "${MODULE_NAME}"; then
        checkpoint_set "rmmod_success" "passed"
        sleep 1
        
        if ! lsmod | grep -q "${MODULE_NAME}"; then
            checkpoint_set "module_unloaded" "passed"
            log_info "Module unloaded successfully"
            return 0
        else
            checkpoint_set "module_unloaded" "failed"
            log_error "Module unloaded but still in lsmod"
            return 1
        fi
    else
        checkpoint_set "rmmod_success" "failed"
        log_error "Failed to unload module"
        return 1
    fi
}

# Reload module
reload_module() {
    unload_module
    sleep 1
    load_module
}

# Check driver status
check_status() {
    log_info "PCIe Test Driver Status"
    log_info "======================="
    
    if lsmod | grep -q "${MODULE_NAME}"; then
        log_info "Module Status: LOADED"
        checkpoint_set "status_check_module" "passed"
        lsmod | grep "${MODULE_NAME}"
    else
        log_info "Module Status: NOT LOADED"
        checkpoint_set "status_check_module" "failed"
    fi
    
    # Check for devices
    local dev_count=0
    for dev in /dev/pcie_test_*; do
        if [[ -e "$dev" ]]; then
            log_info "Found device: $dev"
            ((dev_count++))
        fi
    done
    
    if [[ $dev_count -eq 0 ]]; then
        log_warn "No PCIe test devices found"
        checkpoint_set "status_check_devices" "failed"
    else
        log_info "Total devices: $dev_count"
        checkpoint_set "status_check_devices" "passed"
    fi
    
    # Check kernel messages
    log_info "Recent kernel messages:"
    local msg_count=$(dmesg | grep "${MODULE_NAME}" | wc -l)
    if [[ $msg_count -gt 0 ]]; then
        dmesg | grep "${MODULE_NAME}" | tail -10
        checkpoint_set "status_check_logs" "passed"
    else
        log_warn "No kernel messages found"
        checkpoint_set "status_check_logs" "failed"
    fi
    
    checkpoint_list
}

# Run basic tests
test_basic() {
    log_info "Running basic functionality tests..."
    
    if [[ ! -f "${UTIL_PATH}" ]]; then
        log_error "Test utility not found"
        checkpoint_set "test_utility_exists" "failed"
        return 1
    fi
    checkpoint_set "test_utility_exists" "passed"
    
    if ! lsmod | grep -q "${MODULE_NAME}"; then
        log_error "Module not loaded"
        checkpoint_set "module_running" "failed"
        return 1
    fi
    checkpoint_set "module_running" "passed"
    
    log_info "Test 1: List devices"
    if "${UTIL_PATH}" list > /dev/null 2>&1; then
        log_info "✓ Device listing works"
        checkpoint_set "test_list_devices" "passed"
    else
        log_warn "✗ Device listing failed"
        checkpoint_set "test_list_devices" "failed"
    fi
    
    # Find first available device
    local dev=""
    for d in /dev/pcie_test_*; do
        if [[ -e "$d" ]]; then
            dev="$d"
            break
        fi
    done
    
    if [[ -z "$dev" ]]; then
        log_warn "No PCIe devices available for testing"
        checkpoint_set "device_available" "skipped"
        return 0
    fi
    checkpoint_set "device_available" "passed"
    
    log_info "Test 2: Read device info from $dev"
    if "${UTIL_PATH}" -d "${dev}" info > /dev/null 2>&1; then
        log_info "✓ Device info works"
        checkpoint_set "test_device_info" "passed"
    else
        log_warn "✗ Device info failed"
        checkpoint_set "test_device_info" "failed"
    fi
    
    log_info "Test 3: Read statistics"
    if "${UTIL_PATH}" -d "${dev}" stats > /dev/null 2>&1; then
        log_info "✓ Statistics work"
        checkpoint_set "test_statistics" "passed"
    else
        log_warn "✗ Statistics failed"
        checkpoint_set "test_statistics" "failed"
    fi
    
    log_info "Basic tests completed"
    checkpoint_list
}

# Run all tests
test_all() {
    log_info "Running comprehensive test suite..."
    test_basic
    log_info "All tests completed"
}

# Cleanup
cleanup() {
    log_info "Cleaning up..."
    cd "${SCRIPT_DIR}"
    make clean
    rm -f "${UTIL_PATH}"
    checkpoint_set "cleanup_complete" "passed"
    log_info "Cleanup completed"
}

# Report checkpoints
report_checkpoints() {
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}   PCIe Test Driver - Checkpoint Report${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    
    if [[ ! -f "${TEST_LOG}" ]]; then
        echo -e "${YELLOW}No checkpoints recorded yet${NC}"
        echo "Run some actions first: ./test_driver.sh build"
        return
    fi
    
    echo -e "${GREEN}Recent Checkpoints:${NC}"
    echo "==================="
    grep "CHECKPOINT:" "${TEST_LOG}" | tail -20
    
    echo ""
    echo -e "${GREEN}Checkpoint Statistics:${NC}"
    echo "======================"
    
    local passed=$(grep "CHECKPOINT:.*passed" "${TEST_LOG}" | wc -l)
    local failed=$(grep "CHECKPOINT:.*failed" "${TEST_LOG}" | wc -l)
    local skipped=$(grep "CHECKPOINT:.*skipped" "${TEST_LOG}" | wc -l)
    local total=$((passed + failed + skipped))
    
    echo "Total:   $total"
    echo -e "${GREEN}Passed:  $passed${NC}"
    echo -e "${RED}Failed:  $failed${NC}"
    echo -e "${YELLOW}Skipped: $skipped${NC}"
    
    if [[ $total -gt 0 ]]; then
        local pass_percent=$((passed * 100 / total))
        echo ""
        echo "Success Rate: ${pass_percent}% (${passed}/${total})"
    fi
    
    echo ""
    echo -e "${GREEN}Log File:${NC}"
    echo "$TEST_LOG"
    echo ""
}

# Main
case "${ACTION}" in
    build)
        build
        ;;
    load)
        load_module
        ;;
    unload)
        unload_module
        ;;
    reload)
        reload_module
        ;;
    test-basic)
        test_basic
        ;;
    test-all)
        test_all
        ;;
    check)
        check_status
        ;;
    checkpoints)
        report_checkpoints
        ;;
    cleanup)
        cleanup
        ;;
    *)
        usage
        exit 1
        ;;
esac

exit $?
