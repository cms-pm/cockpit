/*
 * Protocol Context Management
 * 
 * Session lifecycle management for bootloader protocol state.
 * Integrates with Phase 4.5.1 timeout management for reliability.
 */

#include "bootloader_protocol.h"
#include "host_interface/host_interface.h"
#include <string.h>

// Protocol context definitions are now in header file

// Global protocol context
static protocol_context_t g_protocol_context;

void protocol_context_init(protocol_context_t* ctx) {
    if (!ctx) return;
    
    // Initialize flash context
    flash_context_init(&ctx->flash_ctx);
    
    // Initialize protocol state
    ctx->sequence_counter = 0;
    ctx->state = PROTOCOL_STATE_IDLE;
    ctx->session_timeout_ms = 30000;  // 30 second timeout
    ctx->last_activity_time = get_tick_ms();
    
    // Initialize transfer tracking
    ctx->data_received = false;
    ctx->expected_data_length = 0;
    ctx->actual_data_length = 0;
}

bool protocol_is_session_timeout(const protocol_context_t* ctx) {
    if (!ctx) return true;
    
    uint32_t current_time = get_tick_ms();
    uint32_t elapsed;
    
    // Overflow-safe timeout calculation (from Phase 4.5.1)
    if (current_time >= ctx->last_activity_time) {
        elapsed = current_time - ctx->last_activity_time;
    } else {
        elapsed = (UINT32_MAX - ctx->last_activity_time) + current_time + 1;
    }
    
    return elapsed >= ctx->session_timeout_ms;
}

void protocol_update_activity(protocol_context_t* ctx) {
    if (!ctx) return;
    ctx->last_activity_time = get_tick_ms();
}

bootloader_protocol_result_t protocol_reset_session(protocol_context_t* ctx) {
    if (!ctx) return BOOTLOADER_PROTOCOL_ERROR_STATE_INVALID;
    
    // Reset to clean state
    protocol_context_init(ctx);
    
    return BOOTLOADER_PROTOCOL_SUCCESS;
}

// Get global protocol context
protocol_context_t* protocol_get_context(void) {
    return &g_protocol_context;
}

// Initialize global context (called once at bootloader startup)
void protocol_init(void) {
    protocol_context_init(&g_protocol_context);
}