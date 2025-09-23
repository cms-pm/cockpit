#include "vm_memory_pool.h"
#include <cstring>

// Static memory pool - compile-time allocation
VMMemoryContext VMMemoryPool::memory_pool_[MAX_CONCURRENT_VMS];
bool VMMemoryPool::pool_allocated_[MAX_CONCURRENT_VMS] = {false};

VMMemoryContext* VMMemoryPool::acquire_context(uint8_t vm_id) noexcept {
    if (vm_id >= MAX_CONCURRENT_VMS || pool_allocated_[vm_id]) {
        return nullptr;  // Resource exhaustion or already allocated
    }

    pool_allocated_[vm_id] = true;
    memory_pool_[vm_id].reset();  // Initialize to zero state
    return &memory_pool_[vm_id];
}

void VMMemoryPool::release_context(uint8_t vm_id) noexcept {
    if (vm_id < MAX_CONCURRENT_VMS && pool_allocated_[vm_id]) {
        pool_allocated_[vm_id] = false;
        // Security: Clear memory on release
        memory_pool_[vm_id].reset();
    }
}

bool VMMemoryPool::is_allocated(uint8_t vm_id) noexcept {
    if (vm_id >= MAX_CONCURRENT_VMS) {
        return false;
    }
    return pool_allocated_[vm_id];
}

uint8_t VMMemoryPool::get_allocated_count() noexcept {
    uint8_t count = 0;
    for (size_t i = 0; i < MAX_CONCURRENT_VMS; ++i) {
        if (pool_allocated_[i]) {
            count++;
        }
    }
    return count;
}

size_t VMMemoryPool::get_total_memory_usage() noexcept {
    size_t total = 0;
    for (size_t i = 0; i < MAX_CONCURRENT_VMS; ++i) {
        if (pool_allocated_[i]) {
            total += sizeof(VMMemoryContext);
        }
    }
    return total;
}

size_t VMMemoryPool::get_context_memory_usage(uint8_t vm_id) noexcept {
    if (vm_id >= MAX_CONCURRENT_VMS || !pool_allocated_[vm_id]) {
        return 0;
    }
    return sizeof(VMMemoryContext);
}

bool VMMemoryPool::validate_pool_integrity() noexcept {
    for (size_t i = 0; i < MAX_CONCURRENT_VMS; ++i) {
        if (pool_allocated_[i]) {
            if (!memory_pool_[i].validate_integrity()) {
                return false;
            }
        }
    }
    return true;
}

void VMMemoryPool::emergency_reset() noexcept {
    for (size_t i = 0; i < MAX_CONCURRENT_VMS; ++i) {
        pool_allocated_[i] = false;
        memory_pool_[i].reset();
    }
}