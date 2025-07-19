#!/bin/bash
# Development Tools Build Script
# Builds compiler development and validation tools

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
CMAKE_FILE="$SCRIPT_DIR/CMakeLists.txt"

echo "Building ComponentVM Compiler Development Tools..."

# Check for required tools
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake required for building development tools"
    echo "Install CMake or use library-only mode"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "ERROR: Make required for building development tools"
    exit 1
fi

# Check for CMakeLists.txt
if [ ! -f "$CMAKE_FILE" ]; then
    echo "ERROR: CMakeLists.txt not found: $CMAKE_FILE"
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Configuring build..."
cmake ..

echo "Building tools..."
make -j$(nproc 2>/dev/null || echo 4)

echo "Development tools built successfully!"
echo "Available executables:"
find "$BUILD_DIR" -executable -type f -name "*compiler*" -o -name "*validator*" -o -name "*test*" | sort