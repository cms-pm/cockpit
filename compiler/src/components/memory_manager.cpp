#include "memory_manager.h"
#include <algorithm>
#include <cstring>

MemoryManager::MemoryManager() noexcept
    : globals_{}, global_count_(0), array_pool_{}, pool_used_(0), 
      arrays_{}, array_count_(0)
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
    
    // Clear all memory for security (prevent data leakage)
    std::fill(globals_.begin(), globals_.end(), 0);
    std::fill(array_pool_.begin(), array_pool_.end(), 0);
    
    // Reset array descriptors
    for (auto& desc : arrays_) {
        desc.offset = 0;
        desc.size = 0;
        desc.active = false;
    }
}

bool MemoryManager::store_global(uint8_t index, int32_t value) noexcept
{
    if (!is_valid_global_index(index)) {
        return false;
    }
    
    globals_[index] = value;
    
    // Expand global count if needed
    if (index >= global_count_) {
        global_count_ = index + 1;
    }
    
    return true;
}

bool MemoryManager::load_global(uint8_t index, int32_t& value) const noexcept
{
    if (!is_valid_global_index(index)) {
        return false;
    }
    
    value = globals_[index];
    return true;
}

bool MemoryManager::create_array(uint8_t array_id, size_t size) noexcept
{
    if (array_id >= MAX_ARRAYS || size == 0 || size > MAX_ARRAY_SIZE) {
        return false;
    }
    
    // Check if array already exists
    if (arrays_[array_id].active) {
        return false;
    }
    
    // Allocate space in pool
    size_t offset;
    if (!allocate_array_space(size, offset)) {
        return false;
    }
    
    // Initialize array descriptor
    arrays_[array_id].offset = offset;
    arrays_[array_id].size = size;
    arrays_[array_id].active = true;
    
    // Initialize array elements to zero
    std::fill(array_pool_.begin() + offset, array_pool_.begin() + offset + size, 0);
    
    // Update array count
    if (array_id >= array_count_) {
        array_count_ = array_id + 1;
    }
    
    return true;
}

bool MemoryManager::store_array_element(uint8_t array_id, uint16_t index, int32_t value) noexcept
{
    if (!is_valid_array_id(array_id) || !is_valid_array_index(array_id, index)) {
        return false;
    }
    
    const ArrayDescriptor& desc = arrays_[array_id];
    array_pool_[desc.offset + index] = value;
    return true;
}

bool MemoryManager::load_array_element(uint8_t array_id, uint16_t index, int32_t& value) const noexcept
{
    if (!is_valid_array_id(array_id) || !is_valid_array_index(array_id, index)) {
        return false;
    }
    
    const ArrayDescriptor& desc = arrays_[array_id];
    value = array_pool_[desc.offset + index];
    return true;
}

bool MemoryManager::get_array_size(uint8_t array_id, size_t& size) const noexcept
{
    if (!is_valid_array_id(array_id)) {
        return false;
    }
    
    size = arrays_[array_id].size;
    return true;
}

int32_t* MemoryManager::get_array_base(uint8_t array_id) const noexcept
{
    if (!is_valid_array_id(array_id)) {
        return nullptr;
    }
    
    const ArrayDescriptor& desc = arrays_[array_id];
    if (!desc.active) {
        return nullptr;
    }
    
    return const_cast<int32_t*>(&array_pool_[desc.offset]);
}

uint16_t MemoryManager::get_array_size_direct(uint8_t array_id) const noexcept
{
    if (!is_valid_array_id(array_id)) {
        return 0;
    }
    
    const ArrayDescriptor& desc = arrays_[array_id];
    if (!desc.active) {
        return 0;
    }
    
    return static_cast<uint16_t>(desc.size);
}

void MemoryManager::reset() noexcept
{
    // Clear all globals
    std::fill(globals_.begin(), globals_.end(), 0);
    global_count_ = 0;
    
    // Clear all arrays
    std::fill(array_pool_.begin(), array_pool_.end(), 0);
    pool_used_ = 0;
    
    // Reset array descriptors
    for (auto& desc : arrays_) {
        desc.offset = 0;
        desc.size = 0;
        desc.active = false;
    }
    array_count_ = 0;
}

size_t MemoryManager::get_used_array_memory() const noexcept
{
    return pool_used_;
}

size_t MemoryManager::get_available_array_memory() const noexcept
{
    return ARRAY_POOL_SIZE - pool_used_;
}

bool MemoryManager::validate_memory_integrity() const noexcept
{
    #ifdef DEBUG
    // Check for array descriptor consistency
    size_t calculated_used = 0;
    for (uint8_t i = 0; i < array_count_; ++i) {
        if (arrays_[i].active) {
            if (arrays_[i].offset + arrays_[i].size > ARRAY_POOL_SIZE) {
                return false;  // Array bounds exceed pool
            }
            calculated_used += arrays_[i].size;
        }
    }
    
    if (calculated_used != pool_used_) {
        return false;  // Pool usage accounting mismatch
    }
    
    // Additional integrity checks could be added here
    #endif
    
    return true;
}

bool MemoryManager::is_valid_global_index(uint8_t index) const noexcept
{
    return index < MAX_GLOBALS;
}

bool MemoryManager::is_valid_array_id(uint8_t array_id) const noexcept
{
    return array_id < MAX_ARRAYS && arrays_[array_id].active;
}

bool MemoryManager::is_valid_array_index(uint8_t array_id, uint16_t index) const noexcept
{
    if (!is_valid_array_id(array_id)) {
        return false;
    }
    
    return index < arrays_[array_id].size;
}

bool MemoryManager::allocate_array_space(size_t size, size_t& offset) noexcept
{
    if (pool_used_ + size > ARRAY_POOL_SIZE) {
        return false;  // Not enough space
    }
    
    offset = pool_used_;
    pool_used_ += size;
    return true;
}

void MemoryManager::deallocate_array_space(uint8_t array_id) noexcept
{
    if (!is_valid_array_id(array_id)) {
        return;
    }
    
    // Simple deallocation - just mark as inactive
    // In a more sophisticated system, we might implement compaction
    arrays_[array_id].active = false;
    
    // For now, we don't reclaim the space to avoid fragmentation
    // A future enhancement could implement array compaction
}