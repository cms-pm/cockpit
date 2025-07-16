#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RESOURCES 16
#define MAX_CLEANUP_FUNCTIONS 8

typedef enum {
    RESOURCE_TYPE_NONE = 0,
    RESOURCE_TYPE_UART,
    RESOURCE_TYPE_FLASH,
    RESOURCE_TYPE_DMA,
    RESOURCE_TYPE_INTERRUPT,
    RESOURCE_TYPE_TIMER,
    RESOURCE_TYPE_GPIO,
    RESOURCE_TYPE_BUFFER,
    RESOURCE_TYPE_TRANSPORT,
    RESOURCE_TYPE_GENERIC
} resource_type_t;

typedef enum {
    RESOURCE_STATE_UNINITIALIZED = 0,
    RESOURCE_STATE_INITIALIZED,
    RESOURCE_STATE_ACTIVE,
    RESOURCE_STATE_ERROR,
    RESOURCE_STATE_CLEANUP_PENDING,
    RESOURCE_STATE_CLEANED_UP
} resource_state_t;

typedef void (*cleanup_function_t)(void* context);

typedef struct {
    resource_type_t type;
    resource_state_t state;
    void* handle;
    void* context;
    cleanup_function_t cleanup_func;
    const char* name;
    uint32_t init_timestamp;
    uint32_t last_access_timestamp;
    bool critical_resource;
    bool auto_cleanup;
} resource_entry_t;

typedef struct {
    resource_entry_t resources[MAX_RESOURCES];
    cleanup_function_t global_cleanup_functions[MAX_CLEANUP_FUNCTIONS];
    uint8_t resource_count;
    uint8_t cleanup_function_count;
    uint32_t total_allocations;
    uint32_t total_deallocations;
    uint32_t cleanup_failures;
    bool cleanup_in_progress;
    bool emergency_cleanup_mode;
} resource_manager_t;

#define RESOURCE_ENTRY_INIT(resource_type, resource_name, handle_ptr, cleanup_fn, context_ptr) \
    { \
        .type = (resource_type), \
        .state = RESOURCE_STATE_UNINITIALIZED, \
        .handle = (handle_ptr), \
        .context = (context_ptr), \
        .cleanup_func = (cleanup_fn), \
        .name = (resource_name), \
        .init_timestamp = 0, \
        .last_access_timestamp = 0, \
        .critical_resource = false, \
        .auto_cleanup = true \
    }

void resource_manager_init(resource_manager_t* manager);
uint8_t resource_manager_register(resource_manager_t* manager, const resource_entry_t* resource);
bool resource_manager_unregister(resource_manager_t* manager, uint8_t resource_id);

bool resource_manager_add_global_cleanup(resource_manager_t* manager, cleanup_function_t cleanup_func);

void resource_manager_mark_initialized(resource_manager_t* manager, uint8_t resource_id);
void resource_manager_mark_active(resource_manager_t* manager, uint8_t resource_id);
void resource_manager_mark_error(resource_manager_t* manager, uint8_t resource_id);
void resource_manager_touch(resource_manager_t* manager, uint8_t resource_id);

bool resource_manager_cleanup_resource(resource_manager_t* manager, uint8_t resource_id);
void resource_manager_cleanup_all(resource_manager_t* manager);
void resource_manager_cleanup_by_type(resource_manager_t* manager, resource_type_t type);
void resource_manager_emergency_cleanup(resource_manager_t* manager);

uint8_t resource_manager_get_count_by_type(resource_manager_t* manager, resource_type_t type);
uint8_t resource_manager_get_count_by_state(resource_manager_t* manager, resource_state_t state);
bool resource_manager_has_critical_resources(resource_manager_t* manager);
bool resource_manager_has_error_resources(resource_manager_t* manager);

const resource_entry_t* resource_manager_get_resource(resource_manager_t* manager, uint8_t resource_id);
const char* resource_type_to_string(resource_type_t type);
const char* resource_state_to_string(resource_state_t state);

void resource_manager_set_emergency_mode(resource_manager_t* manager, bool emergency);
bool resource_manager_is_emergency_mode(resource_manager_t* manager);

extern resource_manager_t g_resource_manager;

uint32_t get_resource_timestamp(void);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_MANAGER_H