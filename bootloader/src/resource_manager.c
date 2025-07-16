#include "resource_manager.h"
#include <string.h>

resource_manager_t g_resource_manager = {0};
static uint32_t g_resource_timestamp = 0;

uint32_t get_resource_timestamp(void) {
    return ++g_resource_timestamp;
}

void resource_manager_init(resource_manager_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(resource_manager_t));
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        manager->resources[i].state = RESOURCE_STATE_UNINITIALIZED;
        manager->resources[i].type = RESOURCE_TYPE_NONE;
    }
}

uint8_t resource_manager_register(resource_manager_t* manager, const resource_entry_t* resource) {
    if (!manager || !resource || manager->resource_count >= MAX_RESOURCES) {
        return 0xFF;
    }
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].type == RESOURCE_TYPE_NONE) {
            memcpy(&manager->resources[i], resource, sizeof(resource_entry_t));
            manager->resources[i].init_timestamp = get_resource_timestamp();
            manager->resources[i].last_access_timestamp = manager->resources[i].init_timestamp;
            manager->resource_count++;
            manager->total_allocations++;
            return i;
        }
    }
    
    return 0xFF;
}

bool resource_manager_unregister(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) {
        return false;
    }
    
    resource_entry_t* resource = &manager->resources[resource_id];
    
    if (resource->type == RESOURCE_TYPE_NONE) {
        return false;
    }
    
    if (resource->state != RESOURCE_STATE_CLEANED_UP && 
        resource->state != RESOURCE_STATE_UNINITIALIZED) {
        if (!resource_manager_cleanup_resource(manager, resource_id)) {
            return false;
        }
    }
    
    memset(resource, 0, sizeof(resource_entry_t));
    resource->type = RESOURCE_TYPE_NONE;
    resource->state = RESOURCE_STATE_UNINITIALIZED;
    
    if (manager->resource_count > 0) {
        manager->resource_count--;
    }
    
    manager->total_deallocations++;
    
    return true;
}

bool resource_manager_add_global_cleanup(resource_manager_t* manager, cleanup_function_t cleanup_func) {
    if (!manager || !cleanup_func || manager->cleanup_function_count >= MAX_CLEANUP_FUNCTIONS) {
        return false;
    }
    
    manager->global_cleanup_functions[manager->cleanup_function_count] = cleanup_func;
    manager->cleanup_function_count++;
    
    return true;
}

void resource_manager_mark_initialized(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) return;
    
    resource_entry_t* resource = &manager->resources[resource_id];
    if (resource->type != RESOURCE_TYPE_NONE) {
        resource->state = RESOURCE_STATE_INITIALIZED;
        resource->last_access_timestamp = get_resource_timestamp();
    }
}

void resource_manager_mark_active(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) return;
    
    resource_entry_t* resource = &manager->resources[resource_id];
    if (resource->type != RESOURCE_TYPE_NONE) {
        resource->state = RESOURCE_STATE_ACTIVE;
        resource->last_access_timestamp = get_resource_timestamp();
    }
}

void resource_manager_mark_error(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) return;
    
    resource_entry_t* resource = &manager->resources[resource_id];
    if (resource->type != RESOURCE_TYPE_NONE) {
        resource->state = RESOURCE_STATE_ERROR;
        resource->last_access_timestamp = get_resource_timestamp();
    }
}

void resource_manager_touch(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) return;
    
    resource_entry_t* resource = &manager->resources[resource_id];
    if (resource->type != RESOURCE_TYPE_NONE) {
        resource->last_access_timestamp = get_resource_timestamp();
    }
}

bool resource_manager_cleanup_resource(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) {
        return false;
    }
    
    resource_entry_t* resource = &manager->resources[resource_id];
    
    if (resource->type == RESOURCE_TYPE_NONE || 
        resource->state == RESOURCE_STATE_CLEANED_UP) {
        return true;
    }
    
    if (resource->state == RESOURCE_STATE_CLEANUP_PENDING) {
        return false;
    }
    
    resource->state = RESOURCE_STATE_CLEANUP_PENDING;
    
    bool cleanup_success = true;
    
    if (resource->cleanup_func && resource->handle) {
        resource->cleanup_func(resource->context ? resource->context : resource->handle);
    }
    
    if (cleanup_success) {
        resource->state = RESOURCE_STATE_CLEANED_UP;
        resource->last_access_timestamp = get_resource_timestamp();
    } else {
        resource->state = RESOURCE_STATE_ERROR;
        manager->cleanup_failures++;
    }
    
    return cleanup_success;
}

void resource_manager_cleanup_all(resource_manager_t* manager) {
    if (!manager || manager->cleanup_in_progress) {
        return;
    }
    
    manager->cleanup_in_progress = true;
    
    for (uint8_t i = 0; i < manager->cleanup_function_count; i++) {
        if (manager->global_cleanup_functions[i]) {
            manager->global_cleanup_functions[i](NULL);
        }
    }
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].type != RESOURCE_TYPE_NONE && 
            manager->resources[i].auto_cleanup) {
            resource_manager_cleanup_resource(manager, i);
        }
    }
    
    manager->cleanup_in_progress = false;
}

void resource_manager_cleanup_by_type(resource_manager_t* manager, resource_type_t type) {
    if (!manager || manager->cleanup_in_progress) {
        return;
    }
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].type == type) {
            resource_manager_cleanup_resource(manager, i);
        }
    }
}

void resource_manager_emergency_cleanup(resource_manager_t* manager) {
    if (!manager) return;
    
    manager->emergency_cleanup_mode = true;
    manager->cleanup_in_progress = true;
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        resource_entry_t* resource = &manager->resources[i];
        
        if (resource->type != RESOURCE_TYPE_NONE && 
            resource->state != RESOURCE_STATE_CLEANED_UP) {
            
            resource->state = RESOURCE_STATE_CLEANUP_PENDING;
            
            if (resource->cleanup_func && resource->handle) {
                resource->cleanup_func(resource->context ? resource->context : resource->handle);
            }
            
            resource->state = RESOURCE_STATE_CLEANED_UP;
        }
    }
    
    for (uint8_t i = 0; i < manager->cleanup_function_count; i++) {
        if (manager->global_cleanup_functions[i]) {
            manager->global_cleanup_functions[i](NULL);
        }
    }
    
    manager->cleanup_in_progress = false;
}

uint8_t resource_manager_get_count_by_type(resource_manager_t* manager, resource_type_t type) {
    if (!manager) return 0;
    
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].type == type) {
            count++;
        }
    }
    
    return count;
}

uint8_t resource_manager_get_count_by_state(resource_manager_t* manager, resource_state_t state) {
    if (!manager) return 0;
    
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].state == state) {
            count++;
        }
    }
    
    return count;
}

bool resource_manager_has_critical_resources(resource_manager_t* manager) {
    if (!manager) return false;
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].critical_resource && 
            manager->resources[i].type != RESOURCE_TYPE_NONE) {
            return true;
        }
    }
    
    return false;
}

bool resource_manager_has_error_resources(resource_manager_t* manager) {
    if (!manager) return false;
    
    for (uint8_t i = 0; i < MAX_RESOURCES; i++) {
        if (manager->resources[i].state == RESOURCE_STATE_ERROR) {
            return true;
        }
    }
    
    return false;
}

const resource_entry_t* resource_manager_get_resource(resource_manager_t* manager, uint8_t resource_id) {
    if (!manager || resource_id >= MAX_RESOURCES) {
        return NULL;
    }
    
    return &manager->resources[resource_id];
}

const char* resource_type_to_string(resource_type_t type) {
    switch (type) {
        case RESOURCE_TYPE_NONE: return "NONE";
        case RESOURCE_TYPE_UART: return "UART";
        case RESOURCE_TYPE_FLASH: return "FLASH";
        case RESOURCE_TYPE_DMA: return "DMA";
        case RESOURCE_TYPE_INTERRUPT: return "INTERRUPT";
        case RESOURCE_TYPE_TIMER: return "TIMER";
        case RESOURCE_TYPE_GPIO: return "GPIO";
        case RESOURCE_TYPE_BUFFER: return "BUFFER";
        case RESOURCE_TYPE_TRANSPORT: return "TRANSPORT";
        case RESOURCE_TYPE_GENERIC: return "GENERIC";
        default: return "UNKNOWN";
    }
}

const char* resource_state_to_string(resource_state_t state) {
    switch (state) {
        case RESOURCE_STATE_UNINITIALIZED: return "UNINITIALIZED";
        case RESOURCE_STATE_INITIALIZED: return "INITIALIZED";
        case RESOURCE_STATE_ACTIVE: return "ACTIVE";
        case RESOURCE_STATE_ERROR: return "ERROR";
        case RESOURCE_STATE_CLEANUP_PENDING: return "CLEANUP_PENDING";
        case RESOURCE_STATE_CLEANED_UP: return "CLEANED_UP";
        default: return "UNKNOWN";
    }
}

void resource_manager_set_emergency_mode(resource_manager_t* manager, bool emergency) {
    if (!manager) return;
    
    manager->emergency_cleanup_mode = emergency;
}

bool resource_manager_is_emergency_mode(resource_manager_t* manager) {
    if (!manager) return false;
    
    return manager->emergency_cleanup_mode;
}