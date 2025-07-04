/*
 * Simple Button Input System
 * Phase 2, Chunk 2.2: Arduino Input + Button
 * KISS-compliant implementation
 */

#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <stdint.h>
#include <stdbool.h>
#include "../arduino_hal/arduino_hal.h"

// Configuration constants - KISS principle
#define GLOBAL_DEBOUNCE_MS 20        // Single debounce period for all buttons
#define MAX_MONITORED_PINS 4         // Fixed limit, embedded-friendly
#define EVENT_QUEUE_SIZE 8           // Small, fixed event buffer

// Simple button state tracking
typedef struct {
    pin_state_t current_state;       // Current pin reading
    pin_state_t last_stable_state;   // Last confirmed stable state
    uint32_t last_change_time;       // Virtual time of last state change
    bool is_stable;                  // True if state has been stable long enough
} button_state_t;

// Simple button event
typedef struct {
    uint8_t pin;                     // Pin number that generated event
    bool pressed;                    // true = pressed, false = released
    uint32_t timestamp;              // Virtual time when event occurred
} button_event_t;

// Core button functions
void button_init(void);
void button_monitor_pin(uint8_t pin);
void button_update(void);

// Button state reading
pin_state_t button_read_debounced(uint8_t pin);
bool button_pressed(uint8_t pin);
bool button_released(uint8_t pin);

// Event queue (polling only)
bool button_event_available(void);
button_event_t button_event_get(void);

// Virtual timing for QEMU compatibility
uint32_t qemu_get_virtual_time_ms(void);
void qemu_advance_time(uint32_t ms);

// Test helpers for QEMU
void mock_button_press(uint8_t pin);
void mock_button_release(uint8_t pin);
void mock_button_set_state(uint8_t pin, pin_state_t state);

#endif // BUTTON_INPUT_H