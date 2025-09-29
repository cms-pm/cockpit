#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>

// Build-time memory configuration
#ifndef VM_MAX_GLOBALS
#define VM_MAX_GLOBALS 64
#endif

#ifndef VM_MAX_ARRAYS
#define VM_MAX_ARRAYS 16
#endif

#ifndef VM_ARRAY_MAX_ELEMENTS
#define VM_ARRAY_MAX_ELEMENTS 64
#endif

#ifndef MAX_CONCURRENT_VMS
#define MAX_CONCURRENT_VMS 4
#endif

/**
 * @brief Static memory context for ComponentVM instances
 *
 * Provides compile-time deterministic memory allocation with ARM Cortex-M4
 * optimized alignment. Each context provides isolated memory for global
 * variables and multi-dimensional arrays.
 *
 * Memory Layout:
 * - Globals: 64 × 4 bytes = 256 bytes
 * - Arrays: 16 × 64 × 4 bytes = 4,096 bytes
 * - Metadata: ~17 bytes
 * - Total: ~4.3KB per context
 */
struct VMMemoryContext_t {
    // Global variable storage (4-byte aligned for ARM Cortex-M4)
    alignas(4) int32_t globals[VM_MAX_GLOBALS];

    // Hybrid array allocation: pool + descriptors for memory efficiency
    alignas(4) int32_t array_pool[VM_MAX_ARRAYS * VM_ARRAY_MAX_ELEMENTS];  // Total pool

    // Array metadata for efficient allocation
    struct ArrayDescriptor {
        uint16_t offset;        // Offset into array_pool
        uint16_t size;          // Actual array size
        bool active;
        uint8_t padding[1];     // Maintain 4-byte alignment
    } array_descriptors[VM_MAX_ARRAYS];

    // Store the **actual logical capacity** requested by the factory.
    // These per-context limits define the usable portion of the
    // fixed-size arrays.
    // This will likely change once VMs carry config metadata
    // within bytecode files.
    uint8_t actual_global_pool_size = 0;
    uint8_t actual_array_pool_size = 0;
    uint8_t actual_array_max_length = 0;

    // Minimal metadata for runtime management
    uint8_t global_count;
    uint16_t pool_allocated;  // Number of elements allocated in array_pool  

    /*
     * @brief Reset context to initial state
     *
     * Clears all global variables, deactivates all arrays, and resets counters.
     * Used for context reuse and security cleanup.
     */
    void reset() noexcept {
        memset(globals, 0, sizeof(globals));
        memset(array_pool, 0, sizeof(array_pool));
        global_count = 0;
        pool_allocated = 0;
        // Reset all array descriptors
        for (uint8_t i = 0; i < VM_MAX_ARRAYS; ++i) {
            array_descriptors[i] = {0, 0, false, {0}};
        }
    }

    void set_logical_sizes(uint8_t g_size, uint8_t a_size, uint8_t a_max_len) noexcept {
        actual_global_pool_size = g_size;
        actual_array_pool_size = a_size;
        actual_array_max_length = a_max_len;
    }

    /**
     * @brief Get memory usage statistics
     *
     * @return Size in bytes currently allocated for active arrays
     */
    size_t get_array_memory_usage() const noexcept {
        return pool_allocated * sizeof(int32_t);
    }

    /**
     * @brief Get array base pointer from descriptor
     *
     * @param array_id Array identifier
     * @return Pointer to array start, or nullptr if invalid
     */
    int32_t* get_array_ptr(uint8_t array_id) noexcept {
        if (array_id >= VM_MAX_ARRAYS || !array_descriptors[array_id].active) {
            return nullptr;
        }
        return &array_pool[array_descriptors[array_id].offset];
    }

    /**
     * @brief Validate memory context integrity
     *
     * @return true if context is in valid state, false if corruption detected
     */
    bool validate_integrity() const noexcept {
        // Check global count bounds
        if (global_count > VM_MAX_GLOBALS) {
            return false;
        }

        // Array active flags are inherently bounded by array size
        return true;
    }
};

/**
 * @brief Factory function for creating validated VMMemoryContext_t instances
 *
 * Ensures memory constraints are validated and logical sizes are set
 * according to Phase 4.14.1 MemoryManager centralization design.
 *
 * @param global_pool_size Maximum number of global variables (1-64)
 * @param array_pool_size Maximum number of arrays (1-16)
 * @param array_max_length Maximum elements per array (1-64)
 * @return Unique pointer to validated context, or nullptr if invalid parameters
 */
std::unique_ptr<VMMemoryContext_t> VMMemContextFactory(
    uint8_t global_pool_size,
    uint8_t array_pool_size,
    uint8_t array_max_length);
