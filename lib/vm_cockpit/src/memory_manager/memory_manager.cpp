#include "memory_manager.h"
#include <cstring>  // for memset

// Factory function implementation
std::unique_ptr<VMMemoryContext_t> VMMemContextFactory(
    uint8_t global_pool_size,
    uint8_t array_pool_size,
    uint8_t array_max_length) {

    // Validate parameters against compile-time limits
    if (global_pool_size == 0 || global_pool_size > VM_MAX_GLOBALS) {
        return nullptr;
    }
    if (array_pool_size == 0 || array_pool_size > VM_MAX_ARRAYS) {
        return nullptr;
    }
    if (array_max_length == 0 || array_max_length > VM_ARRAY_MAX_ELEMENTS) {
        return nullptr;
    }

    auto context = std::unique_ptr<VMMemoryContext_t>(new VMMemoryContext_t());

    // Set logical constraints before reset
    context->set_logical_sizes(global_pool_size, array_pool_size, array_max_length);

    // Initialize to clean state
    context->reset();

    return context;
}

MemoryManager::MemoryManager() noexcept
    : context_{}, owns_context_{true}
{
    // Create default context with standard limits
    auto default_context = VMMemContextFactory(32, 8, 32);  // 32 globals, 8 arrays, 32 elements each
    if (default_context) {
        context_ = *default_context;
    } else {
        // Fallback to minimal configuration
        context_.reset();
        context_.set_logical_sizes(16, 4, 16);
    }

    #ifdef DEBUG
    stack_canary_enabled_ = true;
    stack_canary_value_ = 0xDEADBEEF;
    #endif
}

MemoryManager::MemoryManager(VMMemoryContext_t context) noexcept
    : context_{context}, owns_context_{true}
{
    #ifdef DEBUG
    stack_canary_enabled_ = true;
    stack_canary_value_ = 0xDEADBEEF;
    #endif
}

MemoryManager::~MemoryManager() noexcept
{
    #ifdef DEBUG
    if (stack_canary_enabled_ && !validate_memory_integrity()) {
        // Memory corruption detected - trigger safety shutdown
        // In real embedded system, this would trigger watchdog reset
    }
    #endif
    
    // Clear context memory for security (prevent data leakage)
    if (owns_context_) {
        context_.reset();  // Clears globals and arrays
    }
}

bool MemoryManager::store_global(uint8_t index, int32_t value) noexcept
{
    if (index >= VM_MAX_GLOBALS) {
        return false;
    }

    context_.globals[index] = value;

    // Expand global count if needed
    if (index >= context_.global_count) {
        context_.global_count = index + 1;
    }

    return true;
}

bool MemoryManager::load_global(uint8_t index, int32_t& value) const noexcept
{
    if (index >= VM_MAX_GLOBALS) {
        return false;
    }

    value = context_.globals[index];
    return true;
}

bool MemoryManager::create_array(uint8_t array_id, size_t size) noexcept
{
    if (array_id >= VM_MAX_ARRAYS || size == 0 || size > VM_ARRAY_MAX_ELEMENTS) {
        return false;
    }

    // Check if array already exists
    if (context_.array_descriptors[array_id].active) {
        return false;
    }

    // Check if we have enough space in the pool
    if (context_.pool_allocated + size > (VM_MAX_ARRAYS * VM_ARRAY_MAX_ELEMENTS)) {
        return false;  // Pool exhausted
    }

    // Allocate from pool
    uint16_t offset = context_.pool_allocated;
    context_.array_descriptors[array_id] = {
        .offset = offset,
        .size = static_cast<uint16_t>(size),
        .active = true,
        .padding = {0}
    };
    context_.pool_allocated += static_cast<uint16_t>(size);

    // Initialize array elements to zero
    memset(&context_.array_pool[offset], 0, size * sizeof(int32_t));

    return true;
}

bool MemoryManager::store_array_element(uint8_t array_id, uint16_t index, int32_t value) noexcept
{
    if (array_id >= VM_MAX_ARRAYS || !context_.array_descriptors[array_id].active ||
        index >= context_.array_descriptors[array_id].size) {
        return false;
    }

    uint16_t pool_index = context_.array_descriptors[array_id].offset + index;
    context_.array_pool[pool_index] = value;
    return true;
}

bool MemoryManager::load_array_element(uint8_t array_id, uint16_t index, int32_t& value) const noexcept
{
    if (array_id >= VM_MAX_ARRAYS || !context_.array_descriptors[array_id].active ||
        index >= context_.array_descriptors[array_id].size) {
        return false;
    }

    uint16_t pool_index = context_.array_descriptors[array_id].offset + index;
    value = context_.array_pool[pool_index];
    return true;
}

bool MemoryManager::get_array_size(uint8_t array_id, size_t& size) const noexcept
{
    if (array_id >= VM_MAX_ARRAYS || !context_.array_descriptors[array_id].active) {
        return false;
    }

    size = context_.array_descriptors[array_id].size;
    return true;
}

int32_t* MemoryManager::get_array_base(uint8_t array_id) const noexcept
{
    if (array_id >= VM_MAX_ARRAYS || !context_.array_descriptors[array_id].active) {
        return nullptr;
    }

    return const_cast<int32_t*>(&context_.array_pool[context_.array_descriptors[array_id].offset]);
}

uint16_t MemoryManager::get_array_size_direct(uint8_t array_id) const noexcept
{
    if (array_id >= VM_MAX_ARRAYS || !context_.array_descriptors[array_id].active) {
        return 0;
    }

    return context_.array_descriptors[array_id].size;
}

void MemoryManager::reset() noexcept
{
    context_.reset();
}

size_t MemoryManager::get_used_array_memory() const noexcept
{
    return context_.pool_allocated * sizeof(int32_t);
}

size_t MemoryManager::get_available_array_memory() const noexcept
{
    size_t total_pool_size = (VM_MAX_ARRAYS * VM_ARRAY_MAX_ELEMENTS) * sizeof(int32_t);
    return total_pool_size - get_used_array_memory();
}

bool MemoryManager::validate_memory_integrity() const noexcept
{
    #ifdef DEBUG
    // Check global count bounds
    if (context_.global_count > VM_MAX_GLOBALS) {
        return false;
    }

    // Check array consistency
    for (uint8_t i = 0; i < VM_MAX_ARRAYS; ++i) {
        if (context_.array_active[i]) {
            // Array is active, no additional checks needed for static allocation
        }
    }

    // Additional integrity checks could be added here
    #endif

    return true;
}

bool MemoryManager::is_valid_global_index(uint8_t index) const noexcept
{
    return index < VM_MAX_GLOBALS;
}

bool MemoryManager::is_valid_array_id(uint8_t array_id) const noexcept
{
    return array_id < VM_MAX_ARRAYS && context_.array_descriptors[array_id].active;
}

bool MemoryManager::is_valid_array_index(uint8_t array_id, uint16_t index) const noexcept
{
    if (!is_valid_array_id(array_id)) {
        return false;
    }

    return index < context_.array_descriptors[array_id].size;
}

bool MemoryManager::allocate_array_space(size_t size, size_t& offset) noexcept
{
    // Check if we have enough space in the pool
    if (context_.pool_allocated + size > (VM_MAX_ARRAYS * VM_ARRAY_MAX_ELEMENTS)) {
        return false;
    }

    offset = context_.pool_allocated;
    return size <= VM_ARRAY_MAX_ELEMENTS;
}

void MemoryManager::deallocate_array_space(uint8_t array_id) noexcept
{
    if (array_id >= VM_MAX_ARRAYS || !context_.array_descriptors[array_id].active) {
        return;
    }

    // Pool-based deallocation - mark as inactive and clear memory
    uint16_t offset = context_.array_descriptors[array_id].offset;
    uint16_t size = context_.array_descriptors[array_id].size;

    context_.array_descriptors[array_id] = {0, 0, false, {0}};
    memset(&context_.array_pool[offset], 0, size * sizeof(int32_t));

    // Note: In MVP, we don't compact the pool (simple implementation)
    // For production, consider implementing pool compaction
}