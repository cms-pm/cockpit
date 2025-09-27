# GT Lite Build System
# Uses existing bridge_c interface for ComponentVM execution

CC = gcc
CXX = g++
CFLAGS = -std=c11 -Wall -Wextra -I../../../lib/vm_cockpit/src -I../../../lib/gt_diagnostics/include -I./include
CXXFLAGS = -std=c++17 -Wall -Wextra -I../../../lib/vm_cockpit/src -I../../../lib/gt_diagnostics/include -I./include

# Phase 4.11.8: ExecutionEngine_v2 conditional compilation support
ifdef USE_EXECUTION_ENGINE_V2
CFLAGS += -DUSE_EXECUTION_ENGINE_V2
CXXFLAGS += -DUSE_EXECUTION_ENGINE_V2
endif

# ComponentVM source files - following vm_compiler pattern
VM_COCKPIT_SRC = ../../../lib/vm_cockpit/src
VM_COMPILER_SRC = ../../../lib/vm_compiler/validation/compiler
GT_DIAG_SRC = ../../../lib/gt_diagnostics/src

# Core ComponentVM dependencies (same as vm_compiler runtime_validator)
ifdef USE_EXECUTION_ENGINE_V2
COMPONENTVM_OBJS = $(VM_COCKPIT_SRC)/component_vm.o \
                   $(VM_COCKPIT_SRC)/execution_engine/execution_engine_v2.o \
                   $(VM_COCKPIT_SRC)/memory_manager/memory_manager.o \
                   $(VM_COCKPIT_SRC)/memory_manager/vm_memory_ops.o \
                   $(VM_COCKPIT_SRC)/memory_manager/vm_memory_pool.o \
                   $(VM_COCKPIT_SRC)/io_controller/io_controller.o \
                   $(VM_COCKPIT_SRC)/vm_errors.o
else
COMPONENTVM_OBJS = $(VM_COCKPIT_SRC)/component_vm.o \
                   $(VM_COCKPIT_SRC)/execution_engine/execution_engine.o \
                   $(VM_COCKPIT_SRC)/execution/vm_handler_registry.o \
                   $(VM_COCKPIT_SRC)/memory_manager/memory_manager.o \
                   $(VM_COCKPIT_SRC)/memory_manager/vm_memory_ops.o \
                   $(VM_COCKPIT_SRC)/memory_manager/vm_memory_pool.o \
                   $(VM_COCKPIT_SRC)/io_controller/io_controller.o \
                   $(VM_COCKPIT_SRC)/vm_errors.o
endif

# Platform stub (use vm_compiler's proven approach + bridge_c compatibility)
PLATFORM_OBJS = $(VM_COMPILER_SRC)/platform_stub.o src/bridge_c_platform_compat.o

# Bridge and diagnostics
BRIDGE_C_OBJS = $(VM_COCKPIT_SRC)/bridge_c/bridge_c.o
GT_DIAG_OBJS = $(GT_DIAG_SRC)/gt_diagnostics.o

# GT Lite runner library
GT_LITE_OBJS = src/gt_lite_runner.o $(BRIDGE_C_OBJS) $(COMPONENTVM_OBJS) $(PLATFORM_OBJS) $(GT_DIAG_OBJS)

# Pattern rule for GT Lite tests
test_lite_%: ../lite_src/test_lite_%.o ../lite_data/test_%.o $(GT_LITE_OBJS)
	@echo "Linking GT Lite test: $@"
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile test source files
../lite_src/test_lite_%.o: ../lite_src/test_lite_%.c
	@if [ ! -f $< ]; then \
		echo "ERROR: Test source $< not found"; \
		exit 2; \
	fi
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test data files
../lite_data/test_%.o: ../lite_data/test_%.c
	@if [ ! -f $< ]; then \
		echo "ERROR: Test data $< not found"; \
		exit 2; \
	fi
	$(CC) $(CFLAGS) -c $< -o $@

# GT Lite runner library
src/gt_lite_runner.o: src/gt_lite_runner.c include/gt_lite_test_types.h
	$(CC) $(CFLAGS) -c $< -o $@

# bridge_c C++ compilation
$(VM_COCKPIT_SRC)/bridge_c/bridge_c.o: $(VM_COCKPIT_SRC)/bridge_c/bridge_c.cpp $(VM_COCKPIT_SRC)/bridge_c/bridge_c.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# gt_diagnostics C compilation
$(GT_DIAG_SRC)/gt_diagnostics.o: $(GT_DIAG_SRC)/gt_diagnostics.c
	$(CC) $(CFLAGS) -c $< -o $@

# ComponentVM C++ compilation rules
$(VM_COCKPIT_SRC)/component_vm.o: $(VM_COCKPIT_SRC)/component_vm.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ifdef USE_EXECUTION_ENGINE_V2
$(VM_COCKPIT_SRC)/execution_engine/execution_engine_v2.o: $(VM_COCKPIT_SRC)/execution_engine/execution_engine_v2.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
else
$(VM_COCKPIT_SRC)/execution_engine/execution_engine.o: $(VM_COCKPIT_SRC)/execution_engine/execution_engine.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
endif

$(VM_COCKPIT_SRC)/memory_manager/memory_manager.o: $(VM_COCKPIT_SRC)/memory_manager/memory_manager.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(VM_COCKPIT_SRC)/memory_manager/vm_memory_ops.o: $(VM_COCKPIT_SRC)/memory_manager/vm_memory_ops.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(VM_COCKPIT_SRC)/memory_manager/vm_memory_pool.o: $(VM_COCKPIT_SRC)/memory_manager/vm_memory_pool.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(VM_COCKPIT_SRC)/io_controller/io_controller.o: $(VM_COCKPIT_SRC)/io_controller/io_controller.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Platform stub C compilation (vm_compiler approach)
$(VM_COMPILER_SRC)/platform_stub.o: $(VM_COMPILER_SRC)/platform_stub.c
	$(CC) $(CFLAGS) -c $< -o $@

# Bridge C platform compatibility layer
src/bridge_c_platform_compat.o: src/bridge_c_platform_compat.c
	$(CC) $(CFLAGS) -c $< -o $@

# VM errors C compilation
$(VM_COCKPIT_SRC)/vm_errors.o: $(VM_COCKPIT_SRC)/vm_errors.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f src/*.o test_lite_*

# Test all available GT Lite tests
test_all:
	@echo "Discovering GT Lite tests..."
	@for test_file in ../lite_src/test_lite_*.c; do \
		if [ -f "$$test_file" ]; then \
			test_name=$$(basename "$$test_file" .c); \
			echo "Building $$test_name..."; \
			$(MAKE) $$test_name || exit 1; \
			echo "Running $$test_name..."; \
			./$$test_name || echo "$$test_name failed"; \
		fi \
	done

.PHONY: clean test_all