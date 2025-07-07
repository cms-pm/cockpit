#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

class IOController {
public:
    static constexpr size_t MAX_STRINGS = 32;
    static constexpr size_t MAX_GPIO_PINS = 20;
    static constexpr size_t STRING_BUFFER_SIZE = 64;
    
    IOController() noexcept;
    ~IOController() noexcept;
    
    // Arduino-compatible digital I/O
    bool digital_write(uint8_t pin, uint8_t value) noexcept;
    bool digital_read(uint8_t pin, uint8_t& value) noexcept;
    bool pin_mode(uint8_t pin, uint8_t mode) noexcept;
    
    // Arduino-compatible analog I/O
    bool analog_write(uint8_t pin, uint16_t value) noexcept;
    bool analog_read(uint8_t pin, uint16_t& value) noexcept;
    
    // Timing functions
    void delay(uint32_t ms) noexcept;
    uint32_t millis() const noexcept;
    uint32_t micros() const noexcept;
    
    // Button/input handling
    bool button_pressed(uint8_t button_id) noexcept;
    bool button_released(uint8_t button_id) noexcept;
    
    // String and printf support
    bool add_string(const char* str, uint8_t& string_id) noexcept;
    bool vm_printf(uint8_t string_id, const int32_t* args, uint8_t arg_count) noexcept;
    
    // Hardware initialization
    bool initialize_hardware() noexcept;
    void reset_hardware() noexcept;
    
    // Pin configuration constants
    enum PinMode : uint8_t {
        INPUT = 0,
        OUTPUT = 1,
        INPUT_PULLUP = 2,
        NO_PULL = 3
    };
    
    // State inspection
    uint8_t get_string_count() const noexcept { return string_count_; }
    bool is_hardware_initialized() const noexcept { return hardware_initialized_; }
    
private:
    // String table for printf support
    std::array<char[STRING_BUFFER_SIZE], MAX_STRINGS> string_table_;
    uint8_t string_count_;
    
    // GPIO state tracking
    struct PinState {
        uint8_t mode;
        uint8_t value;
        bool initialized;
    };
    std::array<PinState, MAX_GPIO_PINS> pin_states_;
    
    // Hardware state
    bool hardware_initialized_;
    
    // Timing base
    uint32_t startup_time_;
    
    // Button state for debouncing
    struct ButtonState {
        bool current;
        bool previous;
        uint32_t last_change;
    };
    std::array<ButtonState, 4> button_states_;
    
    // Hardware abstraction layer functions
    bool hal_digital_write(uint8_t pin, uint8_t value) noexcept;
    bool hal_digital_read(uint8_t pin, uint8_t& value) noexcept;
    bool hal_analog_write(uint8_t pin, uint16_t value) noexcept;
    bool hal_analog_read(uint8_t pin, uint16_t& value) noexcept;
    bool hal_set_pin_mode(uint8_t pin, uint8_t mode) noexcept;
    
    // String management helpers
    bool is_valid_string_id(uint8_t string_id) const noexcept;
    size_t calculate_string_length(const char* str) const noexcept;
    
    // Pin validation helpers
    bool is_valid_pin(uint8_t pin) const noexcept;
    bool is_output_pin(uint8_t pin) const noexcept;
    bool is_input_pin(uint8_t pin) const noexcept;
    
    // Printf formatting helpers
    bool format_printf_string(const char* format, const int32_t* args, 
                             uint8_t arg_count, char* output, size_t output_size) noexcept;
    
    // Disable copy/move
    IOController(const IOController&) = delete;
    IOController& operator=(const IOController&) = delete;
    IOController(IOController&&) = delete;
    IOController& operator=(IOController&&) = delete;
};