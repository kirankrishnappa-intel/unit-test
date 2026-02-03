#!/bin/bash

# Simple script to copy unit-test folder to another machine

set -e

REMOTE_USER="${1:-kirank1}"
REMOTE_HOST="${2:-192.168.1.100}"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}=== Copying unit-test folder ===${NC}"

# Get current directory (should be unit-test)
SOURCE_DIR="$(pwd)"
FOLDER_NAME="$(basename "$SOURCE_DIR")"

echo "Source: $SOURCE_DIR"
echo "Target: ${REMOTE_USER}@${REMOTE_HOST}:~/$FOLDER_NAME"

# Test SSH connection
echo -e "${YELLOW}Testing SSH connection...${NC}"
ssh -q "${REMOTE_USER}@${REMOTE_HOST}" "echo 'OK'" || {
    echo -e "${RED}SSH connection failed${NC}"
    exit 1
}
echo -e "${GREEN}✓ SSH connection OK${NC}"

# Copy the entire unit-test folder
echo -e "${YELLOW}Copying folder...${NC}"
scp -r "$SOURCE_DIR" "${REMOTE_USER}@${REMOTE_HOST}:~/"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Copy successful${NC}"
    echo -e "${YELLOW}Verifying on remote...${NC}"
    ssh "${REMOTE_USER}@${REMOTE_HOST}" "ls -la ~/$FOLDER_NAME/ | head -20"
else
    echo -e "${RED}Copy failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Done!${NC}"
echo -e "${YELLOW}Unit-test folder is now at: ~/$FOLDER_NAME on remote${NC}"
