#include "timeout_manager.h"
#include "host_interface/host_interface.h"
#include <string.h>

timeout_manager_t g_timeout_manager = {0};

uint32_t get_system_tick_safe(void) {
    // Use Host Interface timing for overflow-safe tick management
    return get_tick_ms();
}

static uint32_t calculate_elapsed_safe(uint32_t start_tick, uint32_t current_tick) {
    if (current_tick >= start_tick) {
        return current_tick - start_tick;
    } else {
        return (UINT32_MAX - start_tick) + current_tick + 1;
    }
}

void timeout_manager_init(timeout_manager_t* manager) {
    if (!manager) return;
    
    memset(manager, 0, sizeof(timeout_manager_t));
    manager->last_activity_tick = get_system_tick_safe();
}

uint8_t timeout_manager_register(timeout_manager_t* manager, timeout_context_t* timeout) {
    if (!manager || !timeout || manager->active_count >= MAX_CONCURRENT_TIMEOUTS) {
        return 0xFF;
    }
    
    for (uint8_t i = 0; i < MAX_CONCURRENT_TIMEOUTS; i++) {
        if (manager->timeouts[i].state == TIMEOUT_STATE_DISABLED) {
            memcpy(&manager->timeouts[i], timeout, sizeof(timeout_context_t));
            manager->active_count++;
            return i;
        }
    }
    
    return 0xFF;
}

bool timeout_manager_unregister(timeout_manager_t* manager, uint8_t timeout_id) {
    if (!manager || timeout_id >= MAX_CONCURRENT_TIMEOUTS) {
        return false;
    }
    
    if (manager->timeouts[timeout_id].state != TIMEOUT_STATE_DISABLED) {
        memset(&manager->timeouts[timeout_id], 0, sizeof(timeout_context_t));
        manager->timeouts[timeout_id].state = TIMEOUT_STATE_DISABLED;
        if (manager->active_count > 0) {
            manager->active_count--;
        }
        return true;
    }
    
    return false;
}

void timeout_start(timeout_context_t* timeout) {
    if (!timeout) return;
    
    timeout->start_tick = get_system_tick_safe();
    timeout->state = TIMEOUT_STATE_ACTIVE;
    timeout->timeout_enabled = true;
    timeout->warning_fired = false;
    timeout->retry_count = 0;
}

void timeout_restart(timeout_context_t* timeout) {
    if (!timeout) return;
    
    timeout->start_tick = get_system_tick_safe();
    timeout->state = TIMEOUT_STATE_ACTIVE;
    timeout->warning_fired = false;
}

void timeout_stop(timeout_context_t* timeout) {
    if (!timeout) return;
    
    timeout->timeout_enabled = false;
    timeout->state = TIMEOUT_STATE_DISABLED;
}

void timeout_reset(timeout_context_t* timeout) {
    if (!timeout) return;
    
    timeout->start_tick = get_system_tick_safe();
    timeout->state = TIMEOUT_STATE_ACTIVE;
    timeout->warning_fired = false;
    timeout->retry_count = 0;
}

bool timeout_is_expired(timeout_context_t* timeout) {
    if (!timeout || !timeout->timeout_enabled || timeout->state == TIMEOUT_STATE_DISABLED) {
        return false;
    }
    
    uint32_t current_tick = get_system_tick_safe();
    uint32_t elapsed = calculate_elapsed_safe(timeout->start_tick, current_tick);
    
    if (elapsed >= timeout->timeout_ms) {
        timeout->state = TIMEOUT_STATE_EXPIRED;
        return true;
    }
    
    return false;
}

bool timeout_is_warning(timeout_context_t* timeout) {
    if (!timeout || !timeout->timeout_enabled || timeout->state == TIMEOUT_STATE_DISABLED) {
        return false;
    }
    
    if (timeout->warning_ms == 0) {
        return false;
    }
    
    uint32_t current_tick = get_system_tick_safe();
    uint32_t elapsed = calculate_elapsed_safe(timeout->start_tick, current_tick);
    
    if (elapsed >= timeout->warning_ms && !timeout->warning_fired) {
        timeout->warning_fired = true;
        timeout->state = TIMEOUT_STATE_WARNING;
        return true;
    }
    
    return false;
}

bool timeout_is_active(timeout_context_t* timeout) {
    return timeout && timeout->timeout_enabled && 
           (timeout->state == TIMEOUT_STATE_ACTIVE || timeout->state == TIMEOUT_STATE_WARNING);
}

uint32_t timeout_get_elapsed_ms(timeout_context_t* timeout) {
    if (!timeout || !timeout->timeout_enabled) {
        return 0;
    }
    
    uint32_t current_tick = get_system_tick_safe();
    return calculate_elapsed_safe(timeout->start_tick, current_tick);
}

uint32_t timeout_get_remaining_ms(timeout_context_t* timeout) {
    if (!timeout || !timeout->timeout_enabled) {
        return 0;
    }
    
    uint32_t elapsed = timeout_get_elapsed_ms(timeout);
    
    if (elapsed >= timeout->timeout_ms) {
        return 0;
    }
    
    return timeout->timeout_ms - elapsed;
}

void timeout_configure(timeout_context_t* timeout, uint32_t timeout_ms, uint32_t warning_ms, uint8_t max_retries) {
    if (!timeout) return;
    
    timeout->timeout_ms = timeout_ms;
    timeout->warning_ms = warning_ms;
    timeout->max_retries = max_retries;
}

void timeout_set_auto_reset(timeout_context_t* timeout, bool auto_reset) {
    if (!timeout) return;
    
    timeout->auto_reset_on_activity = auto_reset;
}

void timeout_set_name(timeout_context_t* timeout, const char* name) {
    if (!timeout) return;
    
    timeout->operation_name = name;
}

bool timeout_retry(timeout_context_t* timeout) {
    if (!timeout) return false;
    
    if (timeout->retry_count >= timeout->max_retries) {
        return false;
    }
    
    timeout->retry_count++;
    timeout_restart(timeout);
    
    return true;
}

bool timeout_can_retry(timeout_context_t* timeout) {
    if (!timeout) return false;
    
    return timeout->retry_count < timeout->max_retries;
}

void timeout_manager_update(timeout_manager_t* manager) {
    if (!manager) return;
    
    uint8_t expired_count = 0;
    uint8_t warning_count = 0;
    
    for (uint8_t i = 0; i < MAX_CONCURRENT_TIMEOUTS; i++) {
        timeout_context_t* timeout = &manager->timeouts[i];
        
        if (timeout->state == TIMEOUT_STATE_DISABLED) {
            continue;
        }
        
        if (timeout_is_expired(timeout)) {
            expired_count++;
            manager->total_timeouts++;
        } else if (timeout_is_warning(timeout)) {
            warning_count++;
            manager->total_warnings++;
        }
        
        if (timeout->auto_reset_on_activity) {
            uint32_t current_tick = get_system_tick_safe();
            uint32_t activity_elapsed = calculate_elapsed_safe(manager->last_activity_tick, current_tick);
            
            if (activity_elapsed < 100) {
                timeout_restart(timeout);
            }
        }
    }
}

uint8_t timeout_manager_get_expired_count(timeout_manager_t* manager) {
    if (!manager) return 0;
    
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < MAX_CONCURRENT_TIMEOUTS; i++) {
        if (manager->timeouts[i].state == TIMEOUT_STATE_EXPIRED) {
            count++;
        }
    }
    
    return count;
}

uint8_t timeout_manager_get_warning_count(timeout_manager_t* manager) {
    if (!manager) return 0;
    
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < MAX_CONCURRENT_TIMEOUTS; i++) {
        if (manager->timeouts[i].state == TIMEOUT_STATE_WARNING) {
            count++;
        }
    }
    
    return count;
}

void timeout_manager_record_activity(timeout_manager_t* manager) {
    if (!manager) return;
    
    manager->last_activity_tick = get_system_tick_safe();
}

bool timeout_manager_is_system_responsive(timeout_manager_t* manager, uint32_t max_idle_ms) {
    if (!manager) return false;
    
    uint32_t current_tick = get_system_tick_safe();
    uint32_t idle_time = calculate_elapsed_safe(manager->last_activity_tick, current_tick);
    
    return idle_time < max_idle_ms;
}