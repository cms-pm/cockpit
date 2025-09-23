#include "vm_memory_context.h"

/**
 * @brief Load global variable value
 *
 * @param ctx Pointer to VMMemoryContext
 * @param id Global variable ID (0-63)
 * @param out_value Pointer to store the loaded value
 * @return true if successful, false if invalid parameters
 */
bool vm_load_global(void* ctx, uint8_t id, int32_t* out_value) {
    if (ctx == nullptr || out_value == nullptr || id >= VM_MAX_GLOBALS) {
        return false;
    }

    VMMemoryContext* context = static_cast<VMMemoryContext*>(ctx);
    *out_value = context->globals[id];
    return true;
}

/**
 * @brief Store global variable value
 *
 * @param ctx Pointer to VMMemoryContext
 * @param id Global variable ID (0-63)
 * @param value Value to store
 * @return true if successful, false if invalid parameters
 */
bool vm_store_global(void* ctx, uint8_t id, int32_t value) {
    if (ctx == nullptr || id >= VM_MAX_GLOBALS) {
        return false;
    }

    VMMemoryContext* context = static_cast<VMMemoryContext*>(ctx);
    context->globals[id] = value;

    // Update global count to track highest used global
    if (id >= context->global_count) {
        context->global_count = id + 1;
    }

    return true;
}

/**
 * @brief Create array with specified size
 *
 * @param ctx Pointer to VMMemoryContext
 * @param id Array ID (0-15)
 * @param size Array size in elements (1-64)
 * @return true if successful, false if invalid parameters or array already exists
 */
bool vm_create_array(void* ctx, uint8_t id, size_t size) {
    if (ctx == nullptr || id >= VM_MAX_ARRAYS ||
        size == 0 || size > VM_ARRAY_ELEMENTS) {
        return false;
    }

    VMMemoryContext* context = static_cast<VMMemoryContext*>(ctx);

    // Check if array already exists
    if (context->array_active[id]) {
        return false;
    }

    // Mark array as active and initialize to zero
    context->array_active[id] = true;
    memset(context->arrays[id], 0, size * sizeof(int32_t));

    return true;
}

/**
 * @brief Load array element value
 *
 * @param ctx Pointer to VMMemoryContext
 * @param id Array ID (0-15)
 * @param idx Element index (0-63)
 * @param out_value Pointer to store the loaded value
 * @return true if successful, false if invalid parameters or array not active
 */
bool vm_load_array(void* ctx, uint8_t id, uint16_t idx, int32_t* out_value) {
    if (ctx == nullptr || out_value == nullptr ||
        id >= VM_MAX_ARRAYS || idx >= VM_ARRAY_ELEMENTS) {
        return false;
    }

    VMMemoryContext* context = static_cast<VMMemoryContext*>(ctx);

    // Check if array is active
    if (!context->array_active[id]) {
        return false;
    }

    *out_value = context->arrays[id][idx];
    return true;
}

/**
 * @brief Store array element value
 *
 * @param ctx Pointer to VMMemoryContext
 * @param id Array ID (0-15)
 * @param idx Element index (0-63)
 * @param value Value to store
 * @return true if successful, false if invalid parameters or array not active
 */
bool vm_store_array(void* ctx, uint8_t id, uint16_t idx, int32_t value) {
    if (ctx == nullptr || id >= VM_MAX_ARRAYS || idx >= VM_ARRAY_ELEMENTS) {
        return false;
    }

    VMMemoryContext* context = static_cast<VMMemoryContext*>(ctx);

    // Check if array is active
    if (!context->array_active[id]) {
        return false;
    }

    context->arrays[id][idx] = value;
    return true;
}