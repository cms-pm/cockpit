# Comprehensive Test Suite Makefile
# Phase 3.5: Integration Testing and Validation

BUILD_DIR = ../build
COMPILER = $(BUILD_DIR)/arduino_compiler
TEST_RUNNER = $(BUILD_DIR)/test_runner

.PHONY: all test test-basic test-integration test-complex test-runtime clean setup

all: setup test

setup:
	@echo "=== Setting up test environment ==="
	@cd .. && mkdir -p build && cd build && cmake .. && make

test: test-basic test-integration test-complex
	@echo ""
	@echo "=== PHASE 3 COMPREHENSIVE VALIDATION COMPLETE ==="
	@echo "All test categories executed. Check results above."

test-basic:
	@echo "=== EXECUTING BASIC TESTS ==="
	@cd $(BUILD_DIR) && ./test_runner basic

test-integration:
	@echo ""
	@echo "=== EXECUTING INTEGRATION TESTS ==="
	@cd $(BUILD_DIR) && ./test_runner integration

test-complex:
	@echo ""
	@echo "=== EXECUTING COMPLEX TESTS ==="
	@cd $(BUILD_DIR) && ./test_runner complex

test-all:
	@echo "=== EXECUTING ALL TESTS ==="
	@cd $(BUILD_DIR) && ./test_runner all

test-runtime: test-all
	@echo ""
	@echo "=== EXECUTING BYTECODE RUNTIME VALIDATION ==="
	@echo "Validating compiler→bytecode→VM execution chain..."
	@cd $(BUILD_DIR) && ./runtime_validator

# Individual test execution
test-single:
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make test-single TEST=test_file.c"; \
		exit 1; \
	fi
	@echo "=== Testing single file: $(TEST) ==="
	@cd $(BUILD_DIR) && ./arduino_compiler ../tests/$(TEST)

# Performance validation
test-performance:
	@echo "=== PERFORMANCE VALIDATION ==="
	@echo "Running resource usage analysis..."
	@cd $(BUILD_DIR) && ./test_runner all 2>&1 | grep -E "(HIGH-|bytes|instr)"

# Clean test artifacts
clean:
	@echo "=== Cleaning test artifacts ==="
	@rm -f *.bin *.txt
	@cd $(BUILD_DIR) && rm -f *.bin *.txt compilation_output.txt

# Quick smoke test
smoke:
	@echo "=== SMOKE TEST ==="
	@cd $(BUILD_DIR) && ./arduino_compiler ../tests/test_basic_arithmetic.c
	@echo "✅ Smoke test passed - compiler is functional"

# Help
help:
	@echo "Available targets:"
	@echo "  all           - Run complete test suite"
	@echo "  test          - Run all test categories"
	@echo "  test-basic    - Run basic functionality tests"
	@echo "  test-integration - Run integration tests"
	@echo "  test-complex  - Run complex scenario tests"
	@echo "  test-runtime  - Run bytecode runtime validation"
	@echo "  test-single   - Run single test (usage: make test-single TEST=file.c)"
	@echo "  test-performance - Check resource usage"
	@echo "  smoke         - Quick compiler functionality check"
	@echo "  clean         - Remove test artifacts"
	@echo "  setup         - Build compiler and test runner"