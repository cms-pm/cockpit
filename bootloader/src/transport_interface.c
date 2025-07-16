#include "transport_interface.h"
#include <string.h>

transport_status_t transport_init(transport_context_t* ctx, const transport_interface_t* interface) {
    if (!ctx || !interface) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!interface->init || !interface->send || !interface->receive || 
        !interface->deinit || !interface->get_name) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    memset(ctx, 0, sizeof(transport_context_t));
    ctx->interface = interface;
    ctx->state = TRANSPORT_STATE_UNINITIALIZED;
    
    transport_status_t status = interface->init();
    if (status == TRANSPORT_OK) {
        ctx->state = TRANSPORT_STATE_INITIALIZED;
        ctx->initialized = true;
        
        // Get HAL_GetTick() equivalent - using simple counter for now
        // TODO: Replace with actual HAL_GetTick() when STM32 HAL is available
        static uint32_t tick_counter = 0;
        ctx->init_time = ++tick_counter;
    } else {
        ctx->state = TRANSPORT_STATE_ERROR;
    }
    
    return status;
}

transport_status_t transport_send(transport_context_t* ctx, const uint8_t* data, uint16_t len, uint32_t timeout_ms) {
    if (!ctx || !data || len == 0) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!ctx->initialized || ctx->state == TRANSPORT_STATE_ERROR || 
        ctx->state == TRANSPORT_STATE_SHUTDOWN) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    transport_status_t status = ctx->interface->send(data, len, timeout_ms);
    
    if (status == TRANSPORT_OK) {
        ctx->stats.bytes_sent += len;
        ctx->state = TRANSPORT_STATE_ACTIVE;
    } else {
        ctx->stats.error_count++;
        if (status == TRANSPORT_ERROR_TIMEOUT) {
            ctx->stats.timeout_count++;
        }
    }
    
    return status;
}

transport_status_t transport_receive(transport_context_t* ctx, uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms) {
    if (!ctx || !data || max_len == 0 || !actual_len) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!ctx->initialized || ctx->state == TRANSPORT_STATE_ERROR || 
        ctx->state == TRANSPORT_STATE_SHUTDOWN) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    *actual_len = 0;
    
    transport_status_t status = ctx->interface->receive(data, max_len, actual_len, timeout_ms);
    
    if (status == TRANSPORT_OK) {
        ctx->stats.bytes_received += *actual_len;
        ctx->state = TRANSPORT_STATE_ACTIVE;
    } else {
        ctx->stats.error_count++;
        if (status == TRANSPORT_ERROR_TIMEOUT) {
            ctx->stats.timeout_count++;
        }
    }
    
    return status;
}

transport_status_t transport_available(transport_context_t* ctx, uint16_t* available_bytes) {
    if (!ctx || !available_bytes) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!ctx->initialized || ctx->state == TRANSPORT_STATE_ERROR || 
        ctx->state == TRANSPORT_STATE_SHUTDOWN) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    *available_bytes = 0;
    
    if (ctx->interface->available) {
        return ctx->interface->available(available_bytes);
    }
    
    return TRANSPORT_OK;
}

transport_status_t transport_flush(transport_context_t* ctx) {
    if (!ctx) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!ctx->initialized || ctx->state == TRANSPORT_STATE_ERROR || 
        ctx->state == TRANSPORT_STATE_SHUTDOWN) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    if (ctx->interface->flush) {
        return ctx->interface->flush();
    }
    
    return TRANSPORT_OK;
}

transport_status_t transport_deinit(transport_context_t* ctx) {
    if (!ctx) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!ctx->initialized) {
        return TRANSPORT_OK;
    }
    
    transport_status_t status = TRANSPORT_OK;
    
    if (ctx->interface && ctx->interface->deinit) {
        status = ctx->interface->deinit();
    }
    
    ctx->state = TRANSPORT_STATE_SHUTDOWN;
    ctx->initialized = false;
    
    return status;
}

transport_status_t transport_get_stats(transport_context_t* ctx, transport_stats_t* stats) {
    if (!ctx || !stats) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    memcpy(stats, &ctx->stats, sizeof(transport_stats_t));
    stats->state = ctx->state;
    
    return TRANSPORT_OK;
}

const char* transport_get_name(transport_context_t* ctx) {
    if (!ctx || !ctx->interface || !ctx->interface->get_name) {
        return "unknown";
    }
    
    return ctx->interface->get_name();
}

bool transport_is_initialized(const transport_context_t* ctx) {
    return ctx && ctx->initialized && ctx->state != TRANSPORT_STATE_UNINITIALIZED;
}

bool transport_is_active(const transport_context_t* ctx) {
    return ctx && ctx->initialized && 
           (ctx->state == TRANSPORT_STATE_ACTIVE || ctx->state == TRANSPORT_STATE_INITIALIZED);
}