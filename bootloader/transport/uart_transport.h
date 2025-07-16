#ifndef UART_TRANSPORT_H
#define UART_TRANSPORT_H

#include "transport_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_TRANSPORT_DEFAULT_BAUD 115200
#define UART_TRANSPORT_RX_BUFFER_SIZE 256
#define UART_TRANSPORT_TX_BUFFER_SIZE 256

typedef struct {
    uint32_t baud_rate;
    uint8_t rx_buffer[UART_TRANSPORT_RX_BUFFER_SIZE];
    uint16_t rx_head;
    uint16_t rx_tail;
    uint16_t rx_count;
    bool hardware_initialized;
    uint32_t last_activity_time;
} uart_transport_config_t;

extern const transport_interface_t uart_transport_interface;

transport_status_t uart_transport_configure(uint32_t baud_rate);
transport_status_t uart_transport_get_config(uart_transport_config_t* config);

#ifdef __cplusplus
}
#endif

#endif // UART_TRANSPORT_H