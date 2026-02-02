#!/bin/bash
#
# PCIe Test Driver - Quick Start Guide
#
# This script demonstrates basic usage of the PCIe test driver framework
#

set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}PCIe Test Driver - Quick Start${NC}"
echo -e "${BLUE}========================================${NC}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MODULE_PATH="${SCRIPT_DIR}/pcie_test_driver.ko"
UTIL_PATH="${SCRIPT_DIR}/pcie_test_utility"

# Check if running as root
if [[ $EUID -ne 0 ]]; then
    echo -e "${YELLOW}Note: Some operations require root privileges${NC}"
    echo "Run with: sudo ./QUICKSTART.sh"
fi

echo -e "\n${GREEN}1. Checking Prerequisites${NC}"
echo "   - Kernel version: $(uname -r)"
echo "   - Module path: $MODULE_PATH"
echo "   - Utility path: $UTIL_PATH"

if [[ ! -f "$MODULE_PATH" ]]; then
    echo -e "${YELLOW}   Module not found. Building...${NC}"
    cd "$SCRIPT_DIR"
    make modules > /dev/null 2>&1
    echo "   ✓ Build complete"
fi

if [[ ! -f "$UTIL_PATH" ]]; then
    echo -e "${YELLOW}   Utility not found. Building...${NC}"
    gcc -o "$UTIL_PATH" pcie_test_utility.c -Wall -Wextra > /dev/null 2>&1
    echo "   ✓ Build complete"
fi

echo -e "\n${GREEN}2. Available Commands${NC}"
echo "   List devices:"
echo "     $ $UTIL_PATH list"
echo ""
echo "   Get device info (requires module loaded):"
echo "     $ sudo $SCRIPT_DIR/test_driver.sh load"
echo "     $ $UTIL_PATH -d /dev/pcie_test_0 info"
echo ""
echo "   Read from BAR:"
echo "     $ $UTIL_PATH -d /dev/pcie_test_0 bar-read 0 0x00 32"
echo ""
echo "   Write to BAR:"
echo "     $ $UTIL_PATH -d /dev/pcie_test_0 bar-write 0 0x00 0xDEADBEEF 32"
echo ""
echo "   Read config space:"
echo "     $ $UTIL_PATH -d /dev/pcie_test_0 cfg-read 0x00 16"
echo ""
echo "   Get statistics:"
echo "     $ $UTIL_PATH -d /dev/pcie_test_0 stats"

echo -e "\n${GREEN}3. Test Script Commands${NC}"
echo "   Build:"
echo "     $ ./test_driver.sh build"
echo ""
echo "   Load module:"
echo "     $ sudo ./test_driver.sh load"
echo ""
echo "   Run basic tests:"
echo "     $ sudo ./test_driver.sh test-basic"
echo ""
echo "   Check status:"
echo "     $ sudo ./test_driver.sh check"
echo ""
echo "   Unload module:"
echo "     $ sudo ./test_driver.sh unload"

echo -e "\n${GREEN}4. Quick Test${NC}"

# Check for PCIe devices
DEVICES=$(lspci 2>/dev/null | wc -l)
echo "   Found $DEVICES PCIe devices on system"

# Try to show pcie_test utility usage
echo ""
echo "   Test utility usage:"
"$UTIL_PATH" -h 2>&1 | head -15

echo -e "\n${GREEN}5. Next Steps${NC}"
echo "   1. Read the full README.md for detailed documentation"
echo "   2. Run: sudo ./test_driver.sh load"
echo "   3. Test with: ./pcie_test_utility info"
echo "   4. Check logs: dmesg | grep pcie_test_driver"

echo -e "\n${BLUE}========================================${NC}"
echo -e "${BLUE}For detailed help, see README.md${NC}"
echo -e "${BLUE}========================================${NC}"
