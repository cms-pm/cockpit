# Embedded Hypervisor MVP - Build and Test Automation
# Phase 1, Chunk 1.3: QEMU Integration Foundation

# Configuration
PIO_PATH := /home/chris/.platformio/penv/bin/pio
FIRMWARE := .pio/build/vm_cockpit_qemu/firmware.bin
QEMU_RUNNER := ./scripts/qemu_runner.py

# Ensure PlatformIO is in PATH
export PATH := /home/chris/.platformio/penv/bin:$(PATH)

.PHONY: all build test qemu clean help

# Default target
all: build

# Build the firmware
build:
	@echo "Building embedded hypervisor firmware..."
	$(PIO_PATH) run
	@echo "Build complete: $(FIRMWARE)"

# Run QEMU-compatible tests in QEMU with semihosting output
test: build
	@echo "Running QEMU-compatible VM and GPIO tests..."
	python3 $(QEMU_RUNNER) $(FIRMWARE) --timeout 15
	@echo "Test execution complete"

# Alias for running QEMU tests specifically
test-qemu: test

# Run QEMU with manual monitoring
qemu: build
	@echo "Starting QEMU with monitor access..."
	@echo "Commands: (quit) to exit, (info registers) for CPU state"
	python3 $(QEMU_RUNNER) $(FIRMWARE) --monitor "info registers" --timeout 10

# Check GPIO state (placeholder for future implementation)
check-gpio: build
	@echo "Checking GPIO pin states..."
	python3 $(QEMU_RUNNER) $(FIRMWARE) --check-gpio 13 --timeout 5

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	$(PIO_PATH) run --target clean
	@echo "Clean complete"

# Show help
help:
	@echo "Embedded Hypervisor MVP - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  build      - Compile firmware binary"
	@echo "  test       - Run unit tests in QEMU with semihosting"
	@echo "  qemu       - Start QEMU with monitor access"
	@echo "  check-gpio - Check GPIO pin states (placeholder)"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this message"
	@echo ""
	@echo "Files:"
	@echo "  Firmware:   $(FIRMWARE)"
	@echo "  PlatformIO: $(PIO_PATH)"