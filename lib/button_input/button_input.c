/*
 * Simple Button Input System Implementation
 * Phase 2, Chunk 2.2: Arduino Input + Button
 */

#include "button_input.h"
#include "../semihosting/semihosting.h"
#include "../arduino_hal/arduino_hal.h"

// Global state - fixed arrays for KISS principle
static button_state_t button_states[MAX_MONITORED_PINS];
static uint8_t monitored_pins[MAX_MONITORED_PINS];
static uint8_t num_monitored_pins = 0;

// Event queue - simple circular buffer
static button_event_t event_queue[EVENT_QUEUE_SIZE];
static uint8_t queue_head = 0;
static uint8_t queue_tail = 0;

// Simple virtual time counter for QEMU
static uint32_t virtual_time_ms = 0;

// Initialize button system
void button_init(void) {
    // Reset all state - transient approach
    num_monitored_pins = 0;
    queue_head = 0;
    queue_tail = 0;
    virtual_time_ms = 0;
    
    // Clear all button states
    for (int i = 0; i < MAX_MONITORED_PINS; i++) {
        button_states[i].current_state = PIN_LOW;
        button_states[i].last_stable_state = PIN_LOW;
        button_states[i].last_change_time = 0;
        button_states[i].is_stable = true;
        monitored_pins[i] = 0;
    }
    
#ifdef TESTING
    // Enable mock mode for testing
    hal_enable_mock_mode();
#endif
    
    debug_print("Button system initialized");
}

// Add a pin to monitoring list
void button_monitor_pin(uint8_t pin) {
    if (num_monitored_pins >= MAX_MONITORED_PINS) {
        debug_print("ERROR: Too many monitored pins");
        return;
    }
    
    // Configure pin as input with pullup
    arduino_pin_mode(pin, PIN_MODE_INPUT_PULLUP);
    
    // Add to monitoring list
    monitored_pins[num_monitored_pins] = pin;
    
    // Initialize state
    button_states[num_monitored_pins].current_state = arduino_digital_read(pin);
    button_states[num_monitored_pins].last_stable_state = button_states[num_monitored_pins].current_state;
    button_states[num_monitored_pins].last_change_time = virtual_time_ms;
    button_states[num_monitored_pins].is_stable = true;
    
    num_monitored_pins++;
    
    debug_print_dec("Monitoring pin", pin);
}

// Find button state index for pin
static int find_button_index(uint8_t pin) {
    for (int i = 0; i < num_monitored_pins; i++) {
        if (monitored_pins[i] == pin) {
            return i;
        }
    }
    return -1;
}

// Add event to queue
static void add_event(uint8_t pin, bool pressed) {
    uint8_t next_head = (queue_head + 1) % EVENT_QUEUE_SIZE;
    
    // Check for queue full (drop oldest event)
    if (next_head == queue_tail) {
        queue_tail = (queue_tail + 1) % EVENT_QUEUE_SIZE;
        debug_print("WARNING: Event queue overflow");
    }
    
    // Add new event
    event_queue[queue_head].pin = pin;
    event_queue[queue_head].pressed = pressed;
    event_queue[queue_head].timestamp = virtual_time_ms;
    
    queue_head = next_head;
}

// Update all monitored buttons (call regularly)
void button_update(void) {
    virtual_time_ms++;  // Simple virtual time increment
    
    for (int i = 0; i < num_monitored_pins; i++) {
        uint8_t pin = monitored_pins[i];
        button_state_t *state = &button_states[i];
        
        // Read current pin state
        pin_state_t new_state = arduino_digital_read(pin);
        
        // Check for state change
        if (new_state != state->current_state) {
            // State changed - start debounce period
            state->current_state = new_state;
            state->last_change_time = virtual_time_ms;
            state->is_stable = false;
        } else {
            // State unchanged - check if debounce period elapsed
            if (!state->is_stable && 
                (virtual_time_ms - state->last_change_time) >= GLOBAL_DEBOUNCE_MS) {
                
                // Debounce period complete - state is now stable
                state->is_stable = true;
                
                // Check if this represents a state transition
                if (state->current_state != state->last_stable_state) {
                    // Generate event for state transition
                    bool pressed = (state->current_state == PIN_LOW);  // Assuming active-low buttons
                    add_event(pin, pressed);
                    
                    state->last_stable_state = state->current_state;
                    
                    debug_print_dec(pressed ? "Button pressed" : "Button released", pin);
                }
            }
        }
    }
}

// Get debounced button state
pin_state_t button_read_debounced(uint8_t pin) {
    int index = find_button_index(pin);
    if (index < 0) {
        return PIN_LOW;  // Pin not monitored
    }
    
    button_state_t *state = &button_states[index];
    return state->is_stable ? state->current_state : state->last_stable_state;
}

// Check if button was pressed since last check
bool button_pressed(uint8_t pin) {
    // Simple implementation: check event queue
    for (uint8_t i = queue_tail; i != queue_head; i = (i + 1) % EVENT_QUEUE_SIZE) {
        if (event_queue[i].pin == pin && event_queue[i].pressed) {
            return true;
        }
    }
    return false;
}

// Check if button was released since last check
bool button_released(uint8_t pin) {
    // Simple implementation: check event queue
    for (uint8_t i = queue_tail; i != queue_head; i = (i + 1) % EVENT_QUEUE_SIZE) {
        if (event_queue[i].pin == pin && !event_queue[i].pressed) {
            return true;
        }
    }
    return false;
}

// Check if events are available
bool button_event_available(void) {
    return queue_head != queue_tail;
}

// Get next event from queue
button_event_t button_event_get(void) {
    button_event_t event = {0};
    
    if (button_event_available()) {
        event = event_queue[queue_tail];
        queue_tail = (queue_tail + 1) % EVENT_QUEUE_SIZE;
    }
    
    return event;
}

// Virtual timing functions
uint32_t qemu_get_virtual_time_ms(void) {
    return virtual_time_ms;
}

void qemu_advance_time(uint32_t ms) {
    virtual_time_ms += ms;
}

// Test helpers for QEMU
void mock_button_press(uint8_t pin) {
    mock_button_set_state(pin, PIN_LOW);  // Assuming active-low
    debug_print_dec("Mock button press", pin);
}

void mock_button_release(uint8_t pin) {
    mock_button_set_state(pin, PIN_HIGH);  // Assuming active-low
    debug_print_dec("Mock button release", pin);
}

void mock_button_set_state(uint8_t pin, pin_state_t state) {
#ifdef TESTING
    // Set the mock state at the HAL level so arduino_digital_read() sees it
    hal_set_mock_pin_state(pin, state);
#endif
    debug_print_dec("Mock button state set", pin);
    debug_print_dec("State", state);
}