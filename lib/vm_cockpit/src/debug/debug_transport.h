#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Transport function pointer types
typedef void (*debug_transport_write_fn)(const char* data, size_t length);
typedef bool (*debug_transport_init_fn)(void);
typedef void (*debug_transport_deinit_fn)(void);

// Transport descriptor structure
typedef struct {
    const char* name;
    debug_transport_init_fn init;
    debug_transport_write_fn write;
    debug_transport_deinit_fn deinit;
    bool initialized;
} debug_transport_t;

// Universal debug interface
void debug_printf(const char* format, ...);
void debug_write(const char* data, size_t length);
bool debug_set_transport(debug_transport_t* transport);
debug_transport_t* debug_get_current_transport(void);

// Transport availability checks
bool debug_transport_available(debug_transport_t* transport);
void debug_list_transports(void);

// Built-in transport declarations (implemented in separate files)
extern debug_transport_t itm_transport;
extern debug_transport_t uart_transport;
extern debug_transport_t semihosting_transport;

#ifdef __cplusplus
}
#endif