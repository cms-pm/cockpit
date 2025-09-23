#pragma once

#include "vm_memory_context.h"

/**
 * @brief Static memory pool for ComponentVM instances
 *
 * Manages a fixed pool of VMMemoryContext instances with compile-time
 * deterministic allocation. Provides resource acquisition and release
 * with memory isolation between VM instances.
 *
 * VM ID Assignment:
 * - VM ID 0: SOS Emergency System (highest priority)
 * - VM ID 1: Audio Controller (real-time priority)
 * - VM ID 2: Display Manager (UI priority)
 * - VM ID 3: Debug/Test VM (lowest priority)
 */
class VMMemoryPool {
public:
    /**
     * @brief Acquire memory context for VM instance
     *
     * @param vm_id VM identifier (0-3)
     * @return Pointer to VMMemoryContext if successful, nullptr if unavailable
     */
    static VMMemoryContext* acquire_context(uint8_t vm_id) noexcept;

    /**
     * @brief Release memory context for VM instance
     *
     * @param vm_id VM identifier (0-3)
     */
    static void release_context(uint8_t vm_id) noexcept;

    /**
     * @brief Check if VM context is currently allocated
     *
     * @param vm_id VM identifier (0-3)
     * @return true if allocated, false if available
     */
    static bool is_allocated(uint8_t vm_id) noexcept;

    /**
     * @brief Get total number of allocated contexts
     *
     * @return Number of currently allocated VM contexts
     */
    static uint8_t get_allocated_count() noexcept;

    /**
     * @brief Get total memory usage of allocated contexts
     *
     * @return Total memory usage in bytes
     */
    static size_t get_total_memory_usage() noexcept;

    /**
     * @brief Get memory usage for specific VM context
     *
     * @param vm_id VM identifier (0-3)
     * @return Memory usage in bytes, 0 if not allocated
     */
    static size_t get_context_memory_usage(uint8_t vm_id) noexcept;

    /**
     * @brief Validate integrity of all allocated contexts
     *
     * @return true if all contexts are valid, false if corruption detected
     */
    static bool validate_pool_integrity() noexcept;

    /**
     * @brief Force release all contexts (emergency cleanup)
     *
     * Used for system reset or emergency situations. Clears all allocations
     * and resets memory contexts to zero state.
     */
    static void emergency_reset() noexcept;

private:
    // Static memory pool - compile-time allocation
    static VMMemoryContext memory_pool_[MAX_CONCURRENT_VMS];
    static bool pool_allocated_[MAX_CONCURRENT_VMS];

    // Private constructor - static-only class
    VMMemoryPool() = delete;
    VMMemoryPool(const VMMemoryPool&) = delete;
    VMMemoryPool& operator=(const VMMemoryPool&) = delete;
};