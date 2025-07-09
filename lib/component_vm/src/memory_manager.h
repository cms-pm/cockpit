#pragma once

// #include <array> - removed for embedded compatibility
#include <cstdint>
#include <cstddef>

class MemoryManager {
public:
    static constexpr size_t MAX_GLOBALS = 64;
    static constexpr size_t ARRAY_POOL_SIZE = 2048;
    static constexpr size_t MAX_ARRAYS = 16;
    static constexpr size_t MAX_ARRAY_SIZE = 1024;  // MVP limit: 1024 ints per array
    
    MemoryManager() noexcept;
    ~MemoryManager() noexcept;
    
    // Global variable operations
    bool store_global(uint8_t index, int32_t value) noexcept;
    bool load_global(uint8_t index, int32_t& value) const noexcept;
    uint8_t get_global_count() const noexcept { return global_count_; }
    
    // Array management
    bool create_array(uint8_t array_id, size_t size) noexcept;
    bool store_array_element(uint8_t array_id, uint16_t index, int32_t value) noexcept;
    bool load_array_element(uint8_t array_id, uint16_t index, int32_t& value) const noexcept;
    
    // Array information
    bool get_array_size(uint8_t array_id, size_t& size) const noexcept;
    uint8_t get_array_count() const noexcept { return array_count_; }
    
    // Hybrid approach: Direct access methods for ExecutionEngine
    int32_t* get_array_base(uint8_t array_id) const noexcept;
    uint16_t get_array_size_direct(uint8_t array_id) const noexcept;
    
    // Memory state
    void reset() noexcept;
    size_t get_used_array_memory() const noexcept;
    size_t get_available_array_memory() const noexcept;
    
    // Debug and diagnostics
    bool validate_memory_integrity() const noexcept;
    
private:
    // Global variable storage
    int32_t globals_[MAX_GLOBALS];
    uint8_t global_count_;
    
    // Array storage - static pool allocation
    int32_t array_pool_[ARRAY_POOL_SIZE];
    size_t pool_used_;
    
    // Array metadata
    struct ArrayDescriptor {
        size_t offset;      // Offset into array_pool_
        size_t size;        // Number of elements
        bool active;        // Is this array slot in use
    };
    ArrayDescriptor arrays_[MAX_ARRAYS];
    uint8_t array_count_;
    
    // Memory protection (debug builds)
    #ifdef DEBUG
    bool stack_canary_enabled_;
    uint32_t stack_canary_value_;
    #endif
    
    // Bounds checking helpers
    bool is_valid_global_index(uint8_t index) const noexcept;
    bool is_valid_array_id(uint8_t array_id) const noexcept;
    bool is_valid_array_index(uint8_t array_id, uint16_t index) const noexcept;
    
    // Array allocation helpers
    bool allocate_array_space(size_t size, size_t& offset) noexcept;
    void deallocate_array_space(uint8_t array_id) noexcept;
    
    // Disable copy/move
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator=(MemoryManager&&) = delete;
};