#include "uart_transport.h"
#include "uart_hal.h"
#include <string.h>

static uart_transport_config_t g_uart_config = {0};
static bool g_uart_transport_active = false;

static transport_status_t uart_transport_convert_error(uart_result_t uart_error) {
    switch (uart_error) {
        case UART_SUCCESS:
            return TRANSPORT_OK;
        case UART_ERROR_TIMEOUT:
            return TRANSPORT_ERROR_TIMEOUT;
        case UART_ERROR_HARDWARE:
            return TRANSPORT_ERROR_HARDWARE;
        case UART_ERROR_INVALID_PARAM:
            return TRANSPORT_ERROR_INVALID_PARAM;
        case UART_ERROR_INIT:
        default:
            return TRANSPORT_ERROR_HARDWARE;
    }
}

static transport_status_t uart_transport_init(void) {
    if (g_uart_transport_active) {
        return TRANSPORT_OK;
    }
    
    memset(&g_uart_config, 0, sizeof(g_uart_config));
    g_uart_config.baud_rate = UART_TRANSPORT_DEFAULT_BAUD;
    
    uart_result_t result = uart_init(g_uart_config.baud_rate);
    if (result != UART_SUCCESS) {
        return uart_transport_convert_error(result);
    }
    
    g_uart_config.hardware_initialized = true;
    g_uart_transport_active = true;
    
    const uart_config_t* uart_cfg = uart_get_config();
    if (!uart_cfg || !uart_cfg->initialized) {
        g_uart_transport_active = false;
        return TRANSPORT_ERROR_HARDWARE;
    }
    
    return TRANSPORT_OK;
}

static transport_status_t uart_transport_send(const uint8_t* data, uint16_t len, uint32_t timeout_ms) {
    if (!data || len == 0) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!g_uart_transport_active) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    (void)timeout_ms;
    
    uart_result_t result = uart_write_bytes(data, len);
    return uart_transport_convert_error(result);
}

static transport_status_t uart_transport_receive(uint8_t* data, uint16_t max_len, uint16_t* actual_len, uint32_t timeout_ms) {
    if (!data || max_len == 0 || !actual_len) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!g_uart_transport_active) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    *actual_len = 0;
    
    static uint32_t tick_counter = 0;
    uint32_t start_time = ++tick_counter;
    
    uint16_t bytes_read = 0;
    
    while (bytes_read < max_len) {
        if (uart_data_available()) {
            char c = uart_getchar();
            if (c != 0) {
                data[bytes_read] = (uint8_t)c;
                bytes_read++;
                g_uart_config.last_activity_time = tick_counter;
            }
        }
        
        tick_counter++;
        if (timeout_ms > 0 && (tick_counter - start_time) > timeout_ms) {
            break;
        }
        
        if (bytes_read > 0 && !uart_data_available()) {
            break;
        }
    }
    
    *actual_len = bytes_read;
    
    if (bytes_read == 0 && timeout_ms > 0) {
        return TRANSPORT_ERROR_TIMEOUT;
    }
    
    return TRANSPORT_OK;
}

static transport_status_t uart_transport_available(uint16_t* available_bytes) {
    if (!available_bytes) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    if (!g_uart_transport_active) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    *available_bytes = uart_data_available() ? 1 : 0;
    
    return TRANSPORT_OK;
}

static transport_status_t uart_transport_flush(void) {
    if (!g_uart_transport_active) {
        return TRANSPORT_ERROR_NOT_INITIALIZED;
    }
    
    while (uart_data_available()) {
        uart_getchar();
    }
    
    return TRANSPORT_OK;
}

static transport_status_t uart_transport_deinit(void) {
    if (!g_uart_transport_active) {
        return TRANSPORT_OK;
    }
    
    g_uart_transport_active = false;
    g_uart_config.hardware_initialized = false;
    memset(&g_uart_config, 0, sizeof(g_uart_config));
    
    return TRANSPORT_OK;
}

static transport_status_t uart_transport_get_stats(transport_stats_t* stats) {
    if (!stats) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    memset(stats, 0, sizeof(transport_stats_t));
    
    if (g_uart_transport_active) {
        stats->state = TRANSPORT_STATE_ACTIVE;
    } else {
        stats->state = TRANSPORT_STATE_UNINITIALIZED;
    }
    
    return TRANSPORT_OK;
}

static const char* uart_transport_get_name(void) {
    return "UART-USART1";
}

const transport_interface_t uart_transport_interface = {
    .init = uart_transport_init,
    .send = uart_transport_send,
    .receive = uart_transport_receive,
    .available = uart_transport_available,
    .flush = uart_transport_flush,
    .deinit = uart_transport_deinit,
    .get_stats = uart_transport_get_stats,
    .get_name = uart_transport_get_name
};

transport_status_t uart_transport_configure(uint32_t baud_rate) {
    if (g_uart_transport_active) {
        return TRANSPORT_ERROR_BUSY;
    }
    
    g_uart_config.baud_rate = baud_rate;
    return TRANSPORT_OK;
}

transport_status_t uart_transport_get_config(uart_transport_config_t* config) {
    if (!config) {
        return TRANSPORT_ERROR_INVALID_PARAM;
    }
    
    memcpy(config, &g_uart_config, sizeof(uart_transport_config_t));
    return TRANSPORT_OK;
}