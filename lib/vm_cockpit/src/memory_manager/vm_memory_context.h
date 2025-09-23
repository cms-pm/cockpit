#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

// Build-time memory configuration
#ifndef VM_MAX_GLOBALS
#define VM_MAX_GLOBALS 64
#endif

#ifndef VM_MAX_ARRAYS
#define VM_MAX_ARRAYS 16
#endif

#ifndef VM_ARRAY_ELEMENTS
#define VM_ARRAY_ELEMENTS 64
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
struct VMMemoryContext {
    // Global variable storage (4-byte aligned for ARM Cortex-M4)
    alignas(4) int32_t globals[VM_MAX_GLOBALS];

    // Multi-dimensional array storage (4-byte aligned)
    alignas(4) int32_t arrays[VM_MAX_ARRAYS][VM_ARRAY_ELEMENTS];

    // Minimal metadata for runtime management
    uint8_t global_count;
    bool array_active[VM_MAX_ARRAYS];

    /**
     * @brief Initialize memory context to zero state
     */
    constexpr VMMemoryContext()
        : globals{}, arrays{}, global_count(0), array_active{} {}

    /**
     * @brief Reset context to initial state
     *
     * Clears all global variables, deactivates all arrays, and resets counters.
     * Used for context reuse and security cleanup.
     */
    void reset() noexcept {
        memset(globals, 0, sizeof(globals));
        memset(arrays, 0, sizeof(arrays));
        global_count = 0;
        memset(array_active, false, sizeof(array_active));
    }

    /**
     * @brief Get memory usage statistics
     *
     * @return Size in bytes currently allocated for active arrays
     */
    size_t get_array_memory_usage() const noexcept {
        size_t usage = 0;
        for (size_t i = 0; i < VM_MAX_ARRAYS; ++i) {
            if (array_active[i]) {
                usage += VM_ARRAY_ELEMENTS * sizeof(int32_t);
            }
        }
        return usage;
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
 * @brief Memory operations function pointer interface
 *
 * Provides abstraction layer for memory operations, enabling dependency
 * injection and mock testing. All operations use C-style function pointers
 * for ARM Cortex-M4 optimization.
 */
struct VMMemoryOps {
    // Global variable operations
    bool (*load_global)(void* ctx, uint8_t id, int32_t* out_value);
    bool (*store_global)(void* ctx, uint8_t id, int32_t value);

    // Array operations
    bool (*create_array)(void* ctx, uint8_t id, size_t size);
    bool (*load_array)(void* ctx, uint8_t id, uint16_t idx, int32_t* out_value);
    bool (*store_array)(void* ctx, uint8_t id, uint16_t idx, int32_t value);

    // Context pointer (points to VMMemoryContext)
    void* context;
};

// Memory operation function declarations
extern "C" {
    bool vm_load_global(void* ctx, uint8_t id, int32_t* out_value);
    bool vm_store_global(void* ctx, uint8_t id, int32_t value);
    bool vm_create_array(void* ctx, uint8_t id, size_t size);
    bool vm_load_array(void* ctx, uint8_t id, uint16_t idx, int32_t* out_value);
    bool vm_store_array(void* ctx, uint8_t id, uint16_t idx, int32_t value);
}

/**
 * @brief Create memory operations interface for a context
 *
 * @param context Pointer to VMMemoryContext
 * @return VMMemoryOps structure configured for the context
 */
inline VMMemoryOps create_memory_ops(VMMemoryContext* context) noexcept {
    return {
        vm_load_global,
        vm_store_global,
        vm_create_array,
        vm_load_array,
        vm_store_array,
        context
    };
}