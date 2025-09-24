#include "memory_manager.h"
#include <cstring>  // for memset

MemoryManager::MemoryManager() noexcept
    : context_(&internal_context_), internal_context_{}
{
    context_->reset();  // Initialize to clean state

    #ifdef DEBUG
    stack_canary_enabled_ = true;
    stack_canary_value_ = 0xDEADBEEF;
    #endif
}

MemoryManager::MemoryManager(VMMemoryContext* context) noexcept
    : context_(context), internal_context_{}
{
    if (context_) {
        context_->reset();  // Initialize to clean state
    }

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
    if (context_) {
        context_->reset();  // Clears globals and arrays
    }
}

bool MemoryManager::store_global(uint8_t index, int32_t value) noexcept
{
    if (!context_ || index >= VM_MAX_GLOBALS) {
        return false;
    }

    context_->globals[index] = value;

    // Expand global count if needed
    if (index >= context_->global_count) {
        context_->global_count = index + 1;
    }

    return true;
}

bool MemoryManager::load_global(uint8_t index, int32_t& value) const noexcept
{
    if (!context_ || index >= VM_MAX_GLOBALS) {
        return false;
    }

    value = context_->globals[index];
    return true;
}

bool MemoryManager::create_array(uint8_t array_id, size_t size) noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS || size == 0 || size > VM_ARRAY_ELEMENTS) {
        return false;
    }

    // Check if array already exists
    if (context_->array_active[array_id]) {
        return false;
    }

    // Static array allocation - just mark as active
    context_->array_active[array_id] = true;

    // Initialize array elements to zero
    memset(context_->arrays[array_id], 0, VM_ARRAY_ELEMENTS * sizeof(int32_t));

    return true;
}

bool MemoryManager::store_array_element(uint8_t array_id, uint16_t index, int32_t value) noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS || index >= VM_ARRAY_ELEMENTS ||
        !context_->array_active[array_id]) {
        return false;
    }

    context_->arrays[array_id][index] = value;
    return true;
}

bool MemoryManager::load_array_element(uint8_t array_id, uint16_t index, int32_t& value) const noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS || index >= VM_ARRAY_ELEMENTS ||
        !context_->array_active[array_id]) {
        return false;
    }

    value = context_->arrays[array_id][index];
    return true;
}

bool MemoryManager::get_array_size(uint8_t array_id, size_t& size) const noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS || !context_->array_active[array_id]) {
        return false;
    }

    size = VM_ARRAY_ELEMENTS;  // Static array size in VMMemoryContext
    return true;
}

int32_t* MemoryManager::get_array_base(uint8_t array_id) const noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS || !context_->array_active[array_id]) {
        return nullptr;
    }

    return const_cast<int32_t*>(context_->arrays[array_id]);
}

uint16_t MemoryManager::get_array_size_direct(uint8_t array_id) const noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS || !context_->array_active[array_id]) {
        return 0;
    }

    return VM_ARRAY_ELEMENTS;
}

void MemoryManager::reset() noexcept
{
    if (!context_) {
        return;
    }

    context_->reset();
}

size_t MemoryManager::get_used_array_memory() const noexcept
{
    if (!context_) {
        return 0;
    }

    size_t used = 0;
    for (uint8_t i = 0; i < VM_MAX_ARRAYS; ++i) {
        if (context_->array_active[i]) {
            used += VM_ARRAY_ELEMENTS * sizeof(int32_t);
        }
    }
    return used;
}

size_t MemoryManager::get_available_array_memory() const noexcept
{
    return sizeof(VMMemoryContext) - get_used_array_memory();
}

bool MemoryManager::validate_memory_integrity() const noexcept
{
    #ifdef DEBUG
    if (!context_) {
        return false;
    }

    // Check global count bounds
    if (context_->global_count > VM_MAX_GLOBALS) {
        return false;
    }

    // Check array consistency
    for (uint8_t i = 0; i < VM_MAX_ARRAYS; ++i) {
        if (context_->array_active[i]) {
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
    return context_ && array_id < VM_MAX_ARRAYS && context_->array_active[array_id];
}

bool MemoryManager::is_valid_array_index(uint8_t array_id, uint16_t index) const noexcept
{
    if (!is_valid_array_id(array_id)) {
        return false;
    }

    return index < VM_ARRAY_ELEMENTS;
}

bool MemoryManager::allocate_array_space(size_t size, size_t& offset) noexcept
{
    // Static allocation - no dynamic space management needed
    offset = 0;  // Not used in static allocation
    return size <= VM_ARRAY_ELEMENTS;
}

void MemoryManager::deallocate_array_space(uint8_t array_id) noexcept
{
    if (!context_ || array_id >= VM_MAX_ARRAYS) {
        return;
    }

    // Static deallocation - mark as inactive and clear memory
    context_->array_active[array_id] = false;
    memset(context_->arrays[array_id], 0, VM_ARRAY_ELEMENTS * sizeof(int32_t));
}