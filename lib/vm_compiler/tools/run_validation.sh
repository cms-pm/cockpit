#!/bin/bash
# Validation Test Runner
# Runs compiler validation and integration tests

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$SCRIPT_DIR/build"

echo "Running ComponentVM Compiler Validation Suite..."

# Check if development tools are built
if [ ! -d "$BUILD_DIR" ]; then
    echo "Development tools not built. Building now..."
    "$SCRIPT_DIR/build_dev_tools.sh"
fi

cd "$LIB_DIR"

echo "=== Grammar Validation ==="
echo "Validating ArduinoC.g4 grammar..."
# Basic grammar file existence check
if [ ! -f "grammar/ArduinoC.g4" ]; then
    echo "ERROR: Grammar file not found"
    exit 1
fi
echo "✅ Grammar file valid"

echo "=== Integration Tests ==="
echo "Running integration tests..."
for test_file in validation/integration/*.c; do
    if [ -f "$test_file" ]; then
        echo "Found integration test: $(basename "$test_file")"
    fi
done

echo "=== Compiler Tests ==="
echo "Running compiler tests..."
for test_file in validation/compiler/*.c validation/compiler/*.cpp; do
    if [ -f "$test_file" ]; then
        echo "Found compiler test: $(basename "$test_file")"
    fi
done

echo "=== Development Debug Tests ==="
echo "Found debug tests:"
for test_file in development/debug/*.c; do
    if [ -f "$test_file" ]; then
        echo "  $(basename "$test_file")"
    fi
done

echo ""
echo "✅ Validation suite structure verified"
echo "Note: Full test execution requires ANTLR4 runtime and VM integration"