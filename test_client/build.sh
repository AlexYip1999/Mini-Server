#!/bin/bash

echo "Building Mini Server Test Client..."

# Create build directory
mkdir -p build
cd build

# Configure CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
make -j$(nproc)

# Check if build was successful
if [ -f test_client ]; then
    echo ""
    echo "Build successful! Test client executable created: build/test_client"
    echo ""
    echo "Usage:"
    echo "  ./test_client [host] [port]"
    echo ""
    echo "Examples:"
    echo "  ./test_client                    (connects to localhost:8080)"
    echo "  ./test_client localhost 8080"
    echo "  ./test_client 192.168.1.100 9090"
    echo ""
    echo "To run tests now, make sure the Mini Server is running first, then run:"
    echo "  ./test_client"
else
    echo ""
    echo "Build failed! Please check the error messages above."
    exit 1
fi

cd ..
