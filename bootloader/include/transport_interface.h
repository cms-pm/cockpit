#ifndef TRANSPORT_INTERFACE_H
#define TRANSPORT_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TRANSPORT_OK = 0,
    TRANSPORT_ERROR_TIMEOUT,
    TRANSPORT_ERROR_BUFFER_FULL,
    TRANSPORT_ERROR_HARDWARE,
    TRANSPORT_ERROR_INVALID_PARAM,
    TRANSPORT_ERROR_NOT_INITIALIZED,
    TRANSPORT_ERROR_BUSY
} transport_status_t;

typedef enum {
    TRANSPORT_STATE_UNINITIALIZED = 0,
    TRANSPORT_STATE_INITIALIZED,
    TRANSPORT_STATE_ACTIVE,
    TRANSPORT_STATE_ERROR,
    TRANSPORT_STATE_SHUTDOWN
} transport_state_t;

typedef struct {
    uint32_t bytes_sent;
    uint32_t bytes_received;
    uint32_t error_count;
    uint32_t timeout_count;
    transport_state_t state;
} transport_stats_t;

typedef struct {
    transport_status_t (*init)(void);
    transport_status_t (*send)(const uint8_t* data, uint16_t len, uint32_t timeout_ms);
    transport_status_t (*receive)(uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms);
    transport_status_t (*available)(uint16_t* available_bytes);
    transport_status_t (*flush)(void);
    transport_status_t (*deinit)(void);
    transport_status_t (*get_stats)(transport_stats_t* stats);
    const char* (*get_name)(void);
} transport_interface_t;

typedef struct {
    const transport_interface_t* interface;
    transport_state_t state;
    transport_stats_t stats;
    uint32_t init_time;
    bool initialized;
} transport_context_t;

transport_status_t transport_init(transport_context_t* ctx, const transport_interface_t* interface);
transport_status_t transport_send(transport_context_t* ctx, const uint8_t* data, uint16_t len, uint32_t timeout_ms);
transport_status_t transport_receive(transport_context_t* ctx, uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms);
transport_status_t transport_available(transport_context_t* ctx, uint16_t* available_bytes);
transport_status_t transport_flush(transport_context_t* ctx);
transport_status_t transport_deinit(transport_context_t* ctx);
transport_status_t transport_get_stats(transport_context_t* ctx, transport_stats_t* stats);
const char* transport_get_name(transport_context_t* ctx);

bool transport_is_initialized(const transport_context_t* ctx);
bool transport_is_active(const transport_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif // TRANSPORT_INTERFACE_H