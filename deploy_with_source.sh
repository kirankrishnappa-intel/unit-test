#!/bin/bash

set -e

REMOTE_USER="${1:-kirank1}"
REMOTE_HOST="${2:-192.168.1.100}"
MODULE_NAME="btintel_test_generic_driver"

# Get the script's directory name (unit-test)
SCRIPT_DIR="$(basename "$(pwd)")"

# Get remote user's home directory
REMOTE_HOME=$(ssh "${REMOTE_USER}@${REMOTE_HOST}" "echo ~")

# Create unit-test in remote home directory with same folder name
REMOTE_BASE_PATH="${REMOTE_HOME}/${SCRIPT_DIR}"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== Intel Bluetooth Test Driver Deployment (with source) ===${NC}"

# Step 1: Verify SSH connectivity
echo -e "${YELLOW}[1/5] Checking SSH connectivity...${NC}"
ssh -q "${REMOTE_USER}@${REMOTE_HOST}" "echo 'OK'" || {
    echo -e "${RED}SSH connection failed${NC}"
    exit 1
}
echo -e "${GREEN}✓ SSH connection OK${NC}"

# Step 2: Sync source code
echo -e "${YELLOW}[2/5] Copying unit-test code...${NC}"

LOCAL_DIR="$(pwd)"
SCRIPT_NAME=$(basename "$SCRIPT_DIR")

echo "Local directory: $LOCAL_DIR"
echo "Remote target: ${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_BASE_PATH}"

# Create directory on remote
ssh "${REMOTE_USER}@${REMOTE_HOST}" "mkdir -p ${REMOTE_BASE_PATH}"

# Copy files using scp
echo "Copying files..."
scp -r "${LOCAL_DIR}"/* "${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_BASE_PATH}/" 2>/dev/null || true

# Verify copy
echo "Verifying files..."
ssh "${REMOTE_USER}@${REMOTE_HOST}" "ls -la ${REMOTE_BASE_PATH}/"

echo -e "${GREEN}✓ Files copied${NC}"

# Step 3: Build on remote machine
echo -e "${YELLOW}[3/5] Building on remote machine...${NC}"
ssh "${REMOTE_USER}@${REMOTE_HOST}" << BUILDBUILD
    set -e
    cd "$REMOTE_BASE_PATH"
    echo "Kernel version: \$(uname -r)"
    echo "Building module..."
    make clean
    make
    
    if [ ! -f "${MODULE_NAME}.ko" ]; then
        echo "Build failed!"
        exit 1
    fi
    
    echo "Build completed successfully"
    ls -lh "${MODULE_NAME}.ko"
BUILDBUILD

echo -e "${GREEN}✓ Remote build successful${NC}"

# Step 4: Copy .ko file back to local directory
echo -e "${YELLOW}[4/5] Copying module back to local directory...${NC}"
scp -q "${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_BASE_PATH}/${MODULE_NAME}.ko" "$(pwd)/"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Module copied to: $(pwd)/${MODULE_NAME}.ko${NC}"
else
    echo -e "${RED}Failed to copy module${NC}"
    exit 1
fi

# Step 5: Show kernel logs
echo -e "${YELLOW}[5/5] Kernel logs (last 15 lines):${NC}"
ssh "${REMOTE_USER}@${REMOTE_HOST}" "sudo dmesg | tail -15"

echo -e "${GREEN}✓ Deployment complete!${NC}"
echo -e "${YELLOW}Module is ready at: $(pwd)/${MODULE_NAME}.ko${NC}"
