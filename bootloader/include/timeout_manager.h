#ifndef TIMEOUT_MANAGER_H
#define TIMEOUT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TIMEOUT_STATE_DISABLED = 0,
    TIMEOUT_STATE_ACTIVE,
    TIMEOUT_STATE_WARNING,
    TIMEOUT_STATE_EXPIRED,
    TIMEOUT_STATE_ERROR
} timeout_state_t;

typedef struct {
    uint32_t start_tick;
    uint32_t timeout_ms;
    uint32_t warning_ms;
    uint8_t retry_count;
    uint8_t max_retries;
    timeout_state_t state;
    bool timeout_enabled;
    bool warning_fired;
    bool auto_reset_on_activity;
    const char* operation_name;
} timeout_context_t;

#define MAX_CONCURRENT_TIMEOUTS 8

typedef struct {
    timeout_context_t timeouts[MAX_CONCURRENT_TIMEOUTS];
    uint8_t active_count;
    uint32_t total_timeouts;
    uint32_t total_warnings;
    uint32_t last_activity_tick;
} timeout_manager_t;

#define TIMEOUT_CONTEXT_INIT(name, timeout, warning, retries) \
    { \
        .start_tick = 0, \
        .timeout_ms = (timeout), \
        .warning_ms = (warning), \
        .retry_count = 0, \
        .max_retries = (retries), \
        .state = TIMEOUT_STATE_DISABLED, \
        .timeout_enabled = false, \
        .warning_fired = false, \
        .auto_reset_on_activity = false, \
        .operation_name = (name) \
    }

#define TIMEOUT_INIT_SIMPLE(timeout_ms) \
    TIMEOUT_CONTEXT_INIT("operation", timeout_ms, (timeout_ms * 3) / 4, 3)

void timeout_manager_init(timeout_manager_t* manager);
uint8_t timeout_manager_register(timeout_manager_t* manager, timeout_context_t* timeout);
bool timeout_manager_unregister(timeout_manager_t* manager, uint8_t timeout_id);

void timeout_start(timeout_context_t* timeout);
void timeout_restart(timeout_context_t* timeout);
void timeout_stop(timeout_context_t* timeout);
void timeout_reset(timeout_context_t* timeout);

bool timeout_is_expired(timeout_context_t* timeout);
bool timeout_is_warning(timeout_context_t* timeout);
bool timeout_is_active(timeout_context_t* timeout);
uint32_t timeout_get_elapsed_ms(timeout_context_t* timeout);
uint32_t timeout_get_remaining_ms(timeout_context_t* timeout);

void timeout_configure(timeout_context_t* timeout, uint32_t timeout_ms, uint32_t warning_ms, uint8_t max_retries);
void timeout_set_auto_reset(timeout_context_t* timeout, bool auto_reset);
void timeout_set_name(timeout_context_t* timeout, const char* name);

bool timeout_retry(timeout_context_t* timeout);
bool timeout_can_retry(timeout_context_t* timeout);

void timeout_manager_update(timeout_manager_t* manager);
uint8_t timeout_manager_get_expired_count(timeout_manager_t* manager);
uint8_t timeout_manager_get_warning_count(timeout_manager_t* manager);

void timeout_manager_record_activity(timeout_manager_t* manager);
bool timeout_manager_is_system_responsive(timeout_manager_t* manager, uint32_t max_idle_ms);

extern timeout_manager_t g_timeout_manager;

uint32_t get_system_tick_safe(void);

#ifdef __cplusplus
}
#endif

#endif // TIMEOUT_MANAGER_H