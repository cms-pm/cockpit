#include "io_controller.h"
#include <cstring>
#include <algorithm>

// Platform-specific includes would go here
#ifdef ARDUINO_PLATFORM
#include <Arduino.h>
#elif defined(QEMU_PLATFORM)
#include <stdio.h>
#include <time.h>
#endif

IOController::IOController() noexcept
    : string_table_{}, string_count_(0), pin_states_{}, 
      hardware_initialized_(false), startup_time_(0), button_states_{}
{
    // Initialize pin states
    for (auto& pin : pin_states_) {
        pin.mode = INPUT;
        pin.value = 0;
        pin.initialized = false;
    }
    
    // Initialize button states
    for (auto& button : button_states_) {
        button.current = false;
        button.previous = false;
        button.last_change = 0;
    }
    
    // Record startup time
    startup_time_ = millis();
}

IOController::~IOController() noexcept
{
    if (hardware_initialized_) {
        // Return all pins to safe state (inputs, no pull-up)
        for (uint8_t pin = 0; pin < MAX_GPIO_PINS; ++pin) {
            if (pin_states_[pin].initialized) {
                hal_set_pin_mode(pin, INPUT);
            }
        }
    }
    
    // Clear string table for security
    for (auto& str : string_table_) {
        std::fill(str, str + STRING_BUFFER_SIZE, 0);
    }
}

bool IOController::digital_write(uint8_t pin, uint8_t value) noexcept
{
    if (!is_valid_pin(pin) || !is_output_pin(pin)) {
        return false;
    }
    
    bool success = hal_digital_write(pin, value);
    if (success) {
        pin_states_[pin].value = value;
    }
    
    return success;
}

bool IOController::digital_read(uint8_t pin, uint8_t& value) noexcept
{
    if (!is_valid_pin(pin)) {
        return false;
    }
    
    bool success = hal_digital_read(pin, value);
    if (success) {
        pin_states_[pin].value = value;
    }
    
    return success;
}

bool IOController::pin_mode(uint8_t pin, uint8_t mode) noexcept
{
    if (!is_valid_pin(pin)) {
        return false;
    }
    
    bool success = hal_set_pin_mode(pin, mode);
    if (success) {
        pin_states_[pin].mode = mode;
        pin_states_[pin].initialized = true;
    }
    
    return success;
}

bool IOController::analog_write(uint8_t pin, uint16_t value) noexcept
{
    if (!is_valid_pin(pin) || !is_output_pin(pin)) {
        return false;
    }
    
    return hal_analog_write(pin, value);
}

bool IOController::analog_read(uint8_t pin, uint16_t& value) noexcept
{
    if (!is_valid_pin(pin)) {
        return false;
    }
    
    return hal_analog_read(pin, value);
}

void IOController::delay(uint32_t ms) noexcept
{
    #ifdef ARDUINO_PLATFORM
    ::delay(ms);
    #elif defined(QEMU_PLATFORM)
    // QEMU semihosting delay simulation
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, nullptr);
    #else
    // Busy wait fallback (not ideal for real embedded)
    uint32_t start = millis();
    while (millis() - start < ms) {
        // Busy wait
    }
    #endif
}

uint32_t IOController::millis() const noexcept
{
    #ifdef ARDUINO_PLATFORM
    return ::millis();
    #elif defined(QEMU_PLATFORM)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
    #else
    // Fallback implementation
    return 0;
    #endif
}

uint32_t IOController::micros() const noexcept
{
    #ifdef ARDUINO_PLATFORM
    return ::micros();
    #elif defined(QEMU_PLATFORM)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
    #else
    // Fallback implementation
    return 0;
    #endif
}

bool IOController::button_pressed(uint8_t button_id) noexcept
{
    if (button_id >= 4) {
        return false;
    }
    
    // Simple debouncing logic
    uint32_t current_time = millis();
    uint8_t pin_value;
    
    if (!digital_read(button_id + 2, pin_value)) {  // Buttons on pins 2-5
        return false;
    }
    
    bool current_state = (pin_value == 0);  // Active low
    ButtonState& button = button_states_[button_id];
    
    if (current_state != button.current) {
        if (current_time - button.last_change > 50) {  // 50ms debounce
            button.previous = button.current;
            button.current = current_state;
            button.last_change = current_time;
            
            return current_state && !button.previous;  // Rising edge
        }
    }
    
    return false;
}

bool IOController::button_released(uint8_t button_id) noexcept
{
    if (button_id >= 4) {
        return false;
    }
    
    // Similar to button_pressed but for falling edge
    uint32_t current_time = millis();
    uint8_t pin_value;
    
    if (!digital_read(button_id + 2, pin_value)) {
        return false;
    }
    
    bool current_state = (pin_value == 0);
    ButtonState& button = button_states_[button_id];
    
    if (current_state != button.current) {
        if (current_time - button.last_change > 50) {
            button.previous = button.current;
            button.current = current_state;
            button.last_change = current_time;
            
            return !current_state && button.previous;  // Falling edge
        }
    }
    
    return false;
}

bool IOController::add_string(const char* str, uint8_t& string_id) noexcept
{
    if (string_count_ >= MAX_STRINGS || str == nullptr) {
        return false;
    }
    
    size_t len = calculate_string_length(str);
    if (len >= STRING_BUFFER_SIZE) {
        return false;
    }
    
    // Copy string to table
    std::strncpy(string_table_[string_count_], str, STRING_BUFFER_SIZE - 1);
    string_table_[string_count_][STRING_BUFFER_SIZE - 1] = '\0';
    
    string_id = string_count_;
    string_count_++;
    
    return true;
}

bool IOController::vm_printf(uint8_t string_id, const int32_t* args, uint8_t arg_count) noexcept
{
    if (!is_valid_string_id(string_id)) {
        return false;
    }
    
    const char* format = string_table_[string_id];
    char output_buffer[256];
    
    if (!format_printf_string(format, args, arg_count, output_buffer, sizeof(output_buffer))) {
        return false;
    }
    
    #ifdef ARDUINO_PLATFORM
    Serial.print(output_buffer);
    #elif defined(QEMU_PLATFORM)
    printf("%s", output_buffer);
    #else
    // Fallback - could write to debug buffer
    #endif
    
    return true;
}

bool IOController::initialize_hardware() noexcept
{
    if (hardware_initialized_) {
        return true;
    }
    
    #ifdef ARDUINO_PLATFORM
    // Initialize Arduino hardware
    // Serial already initialized by Arduino framework
    #elif defined(QEMU_PLATFORM)
    // QEMU initialization
    printf("VM IOController initialized\n");
    #endif
    
    hardware_initialized_ = true;
    return true;
}

void IOController::reset_hardware() noexcept
{
    // Reset all pins to safe state
    for (uint8_t pin = 0; pin < MAX_GPIO_PINS; ++pin) {
        if (pin_states_[pin].initialized) {
            hal_set_pin_mode(pin, INPUT);
            pin_states_[pin].initialized = false;
        }
    }
    
    hardware_initialized_ = false;
}

bool IOController::hal_digital_write(uint8_t pin, uint8_t value) noexcept
{
    #ifdef ARDUINO_PLATFORM
    digitalWrite(pin, value);
    return true;
    #elif defined(QEMU_PLATFORM)
    printf("Digital write: pin %d = %d\n", pin, value);
    return true;
    #else
    return false;
    #endif
}

bool IOController::hal_digital_read(uint8_t pin, uint8_t& value) noexcept
{
    #ifdef ARDUINO_PLATFORM
    value = digitalRead(pin);
    return true;
    #elif defined(QEMU_PLATFORM)
    value = 0;  // Simulate low input
    return true;
    #else
    return false;
    #endif
}

bool IOController::hal_analog_write(uint8_t pin, uint16_t value) noexcept
{
    #ifdef ARDUINO_PLATFORM
    analogWrite(pin, value);
    return true;
    #elif defined(QEMU_PLATFORM)
    printf("Analog write: pin %d = %d\n", pin, value);
    return true;
    #else
    return false;
    #endif
}

bool IOController::hal_analog_read(uint8_t pin, uint16_t& value) noexcept
{
    #ifdef ARDUINO_PLATFORM
    value = analogRead(pin);
    return true;
    #elif defined(QEMU_PLATFORM)
    value = 512;  // Simulate mid-scale reading
    return true;
    #else
    return false;
    #endif
}

bool IOController::hal_set_pin_mode(uint8_t pin, uint8_t mode) noexcept
{
    #ifdef ARDUINO_PLATFORM
    pinMode(pin, mode);
    return true;
    #elif defined(QEMU_PLATFORM)
    printf("Pin mode: pin %d = %d\n", pin, mode);
    return true;
    #else
    return false;
    #endif
}

bool IOController::is_valid_string_id(uint8_t string_id) const noexcept
{
    return string_id < string_count_;
}

size_t IOController::calculate_string_length(const char* str) const noexcept
{
    if (str == nullptr) {
        return 0;
    }
    
    size_t len = 0;
    while (str[len] != '\0' && len < STRING_BUFFER_SIZE) {
        len++;
    }
    
    return len;
}

bool IOController::is_valid_pin(uint8_t pin) const noexcept
{
    return pin < MAX_GPIO_PINS;
}

bool IOController::is_output_pin(uint8_t pin) const noexcept
{
    return is_valid_pin(pin) && pin_states_[pin].mode == OUTPUT;
}

bool IOController::is_input_pin(uint8_t pin) const noexcept
{
    return is_valid_pin(pin) && (pin_states_[pin].mode == INPUT || pin_states_[pin].mode == INPUT_PULLUP);
}

bool IOController::format_printf_string(const char* format, const int32_t* args, 
                                       uint8_t arg_count, char* output, size_t output_size) noexcept
{
    if (format == nullptr || output == nullptr || output_size == 0) {
        return false;
    }
    
    // Simple printf implementation supporting %d, %s, %x, %c
    size_t output_pos = 0;
    uint8_t arg_index = 0;
    
    for (size_t i = 0; format[i] != '\0' && output_pos < output_size - 1; ++i) {
        if (format[i] == '%' && format[i + 1] != '\0') {
            if (arg_index >= arg_count) {
                return false;  // Not enough arguments
            }
            
            switch (format[i + 1]) {
                case 'd': {
                    // Integer formatting
                    int32_t value = args[arg_index++];
                    char temp[12];
                    snprintf(temp, sizeof(temp), "%d", value);
                    
                    for (size_t j = 0; temp[j] != '\0' && output_pos < output_size - 1; ++j) {
                        output[output_pos++] = temp[j];
                    }
                    i++;  // Skip format specifier
                    break;
                }
                
                case 'x': {
                    // Hexadecimal formatting
                    int32_t value = args[arg_index++];
                    char temp[12];
                    snprintf(temp, sizeof(temp), "%x", value);
                    
                    for (size_t j = 0; temp[j] != '\0' && output_pos < output_size - 1; ++j) {
                        output[output_pos++] = temp[j];
                    }
                    i++;
                    break;
                }
                
                case 'c': {
                    // Character formatting
                    char c = static_cast<char>(args[arg_index++]);
                    output[output_pos++] = c;
                    i++;
                    break;
                }
                
                default:
                    // Unknown format specifier, just copy
                    output[output_pos++] = format[i];
                    break;
            }
        } else {
            output[output_pos++] = format[i];
        }
    }
    
    output[output_pos] = '\0';
    return true;
}