#!/bin/bash

echo "Mini Server Test Client - Quick Runner"
echo "======================================"

# Check if executable exists
if [ -f build/test_client ]; then
    echo "Found test client executable, running tests..."
    echo ""
    ./build/test_client
else
    echo "Test client not found! Please build it first by running ./build.sh"
    echo ""
    exit 1
fi

echo ""
echo "Tests completed."
