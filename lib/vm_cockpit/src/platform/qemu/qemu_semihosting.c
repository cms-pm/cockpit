/*
 * QEMU Semihosting Implementation
 * Virtual Hardware Control Implementation via ARM Semihosting
 */

#include "qemu_semihosting.h"
#include <stddef.h>

#ifdef QEMU_PLATFORM

// =================================================================
// Virtual Hardware State (QEMU's "Registers")
// =================================================================

static struct {
    bool gpio_states[QEMU_MAX_GPIO_PINS];
    bool gpio_directions[QEMU_MAX_GPIO_PINS]; // true = output, false = input
    bool initialized;
    uint32_t boot_time_ms;
} qemu_virtual_hw = {0};

// =================================================================
// ARM Semihosting Implementation
// =================================================================

/**
 * @brief Perform ARM semihosting call
 * Uses inline assembly to execute the semihosting breakpoint
 */
uint32_t semihost_call(uint32_t operation, void* parameter) {
    uint32_t result;
    
    // ARM semihosting uses specific breakpoint instruction
    // Operation in r0, parameter in r1, result returned in r0
    __asm__ volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "bkpt #0xAB\n"
        "mov %0, r0\n"
        : "=r" (result)
        : "r" (operation), "r" (parameter)
        : "r0", "r1", "memory"
    );
    
    return result;
}

// =================================================================
// QEMU Semihosting HAL Implementation
// =================================================================

void qemu_semihost_init(void) {
    qemu_virtual_hw.initialized = true;
    qemu_virtual_hw.boot_time_ms = qemu_semihost_get_time_ms();
    
    // Initialize virtual GPIO
    qemu_gpio_init();
    
    // Send initialization message
    qemu_semihost_puts("QEMU Semihosting HAL Initialized\n");
}

void qemu_semihost_putchar(char c) {
    semihost_call(SYS_WRITEC, &c);
}

void qemu_semihost_puts(const char* str) {
    if (str == NULL) return;
    semihost_call(SYS_WRITE0, (void*)str);
}

void qemu_semihost_write(const uint8_t* data, uint32_t size) {
    if (data == NULL || size == 0) return;
    
    // SYS_WRITE parameter block: [file_handle, buffer, length]
    uint32_t params[3] = {
        1,              // stdout file handle
        (uint32_t)data, // buffer pointer
        size            // length
    };
    
    semihost_call(SYS_WRITE, params);
}

int qemu_semihost_getchar(void) {
    // Try to read a character (non-blocking in most QEMU setups)
    char c;
    uint32_t params[3] = {
        0,           // stdin file handle
        (uint32_t)&c, // buffer pointer
        1            // length
    };
    
    uint32_t result = semihost_call(SYS_READ, params);
    
    // If read successful, return character, otherwise -1
    return (result == 0) ? (int)c : -1;
}

uint32_t qemu_semihost_get_time_ms(void) {
    // Get system time in centiseconds, convert to milliseconds
    uint32_t centiseconds = semihost_call(SYS_CLOCK, NULL);
    return centiseconds * 10; // Convert to milliseconds
}

void qemu_semihost_delay_ms(uint32_t ms) {
    // Simple software delay - not precise in QEMU but functional
    uint32_t start = qemu_semihost_get_time_ms();
    while ((qemu_semihost_get_time_ms() - start) < ms) {
        // Busy wait - in real QEMU this advances simulation time
    }
}

void qemu_semihost_exit(uint32_t exit_code) {
    semihost_call(SYS_EXIT, &exit_code);
}

// =================================================================
// QEMU Virtual GPIO Implementation
// =================================================================

void qemu_gpio_init(void) {
    // Initialize all GPIO pins to input, low state
    for (int i = 0; i < QEMU_MAX_GPIO_PINS; i++) {
        qemu_virtual_hw.gpio_states[i] = false;
        qemu_virtual_hw.gpio_directions[i] = false; // input
    }
}

void qemu_gpio_set_pin(uint8_t pin, bool state) {
    if (pin >= QEMU_MAX_GPIO_PINS) return;
    
    qemu_virtual_hw.gpio_states[pin] = state;
    
    // Log GPIO changes for debugging
    if (qemu_virtual_hw.gpio_directions[pin]) { // Only log outputs
        char msg[64];
        // Simple sprintf equivalent for basic formatting
        const char* state_str = state ? "HIGH" : "LOW";
        qemu_semihost_puts("QEMU GPIO: Pin ");
        qemu_semihost_putchar('0' + (pin / 10));
        qemu_semihost_putchar('0' + (pin % 10));
        qemu_semihost_puts(" set to ");
        qemu_semihost_puts(state_str);
        qemu_semihost_putchar('\n');
    }
}

bool qemu_gpio_get_pin(uint8_t pin) {
    if (pin >= QEMU_MAX_GPIO_PINS) return false;
    return qemu_virtual_hw.gpio_states[pin];
}

void qemu_gpio_set_direction(uint8_t pin, bool output) {
    if (pin >= QEMU_MAX_GPIO_PINS) return;
    qemu_virtual_hw.gpio_directions[pin] = output;
}

#endif // QEMU_PLATFORM