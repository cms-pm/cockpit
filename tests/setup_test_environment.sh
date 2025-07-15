#!/bin/bash
# ComponentVM Test System Environment Setup
# Sets up Python virtual environment and dependencies

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "🔧 ComponentVM Test System Setup"
echo "=================================="
echo ""

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Error: Python 3 is required but not found"
    echo "Please install Python 3.8 or later"
    exit 1
fi

PYTHON_VERSION=$(python3 --version | cut -d' ' -f2)
echo "✓ Found Python $PYTHON_VERSION"

# Create virtual environment if it doesn't exist
if [ ! -d "test_venv" ]; then
    echo "📦 Creating Python virtual environment..."
    python3 -m venv test_venv
    echo "✓ Virtual environment created"
else
    echo "✓ Virtual environment already exists"
fi

# Activate virtual environment
echo "🔄 Activating virtual environment..."
source test_venv/bin/activate

# Upgrade pip
echo "📈 Upgrading pip..."
pip install --upgrade pip

# Install dependencies
echo "📥 Installing test system dependencies..."
pip install -r requirements.txt

echo ""
echo "✅ Test System Setup Complete!"
echo ""
echo "🚀 Quick Start:"
echo "   ./tools/list_tests           # Show available tests"
echo "   ./tools/run_test <test>      # Run a test"
echo "   ./tools/debug_test <test>    # Debug a test interactively"
echo ""
echo "📋 Environment:"
echo "   Virtual Environment: test_venv/"
echo "   Python: $(python3 --version)"
echo "   Dependencies: $(pip list | wc -l) packages installed"
echo ""