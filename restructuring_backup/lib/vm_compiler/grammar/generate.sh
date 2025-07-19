#!/bin/bash
# ANTLR4 Grammar Generation Script
# Generates C++ parser files from ArduinoC.g4 grammar

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GRAMMAR_FILE="$SCRIPT_DIR/ArduinoC.g4"
ANTLR_JAR="$SCRIPT_DIR/antlr-4.13.1-complete.jar"
OUTPUT_DIR="$SCRIPT_DIR/../src/generated"

# Check for Java
if ! command -v java &> /dev/null; then
    echo "ERROR: Java required for grammar generation"
    echo "Install Java or use pre-built library version"
    exit 1
fi

# Check for grammar file
if [ ! -f "$GRAMMAR_FILE" ]; then
    echo "ERROR: Grammar file not found: $GRAMMAR_FILE"
    exit 1
fi

# Check for ANTLR4 JAR
if [ ! -f "$ANTLR_JAR" ]; then
    echo "ERROR: ANTLR4 JAR not found: $ANTLR_JAR"
    exit 1
fi

echo "Generating ANTLR4 parser from $GRAMMAR_FILE..."

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Generate parser files
java -jar "$ANTLR_JAR" -Dlanguage=Cpp -visitor -o "$OUTPUT_DIR" "$GRAMMAR_FILE"

echo "Grammar generated successfully in $OUTPUT_DIR"
echo "Generated files:"
ls -la "$OUTPUT_DIR"