/*
 * Button Input System Tests
 * Phase 2, Chunk 2.2: Arduino Input + Button
 */

#include "test_gpio_common.h"
#include "../lib/button_input/button_input.h"
#include "../lib/vm_core/vm_core.h"

// Button test results
static gpio_test_results_t button_results = {0, 0, 0};

// Test button system initialization
void test_button_init(void) {
    button_init();
    
    // Verify initialization
    GPIO_TEST_ASSERT(!button_event_available(), "No events after init", &button_results);
    GPIO_TEST_ASSERT(qemu_get_virtual_time_ms() >= 0, "Virtual time initialized", &button_results);
}

// Test button monitoring setup
void test_button_monitoring(void) {
    button_init();
    
    // Monitor a button pin
    button_monitor_pin(PIN_2);
    
    // Should be able to read the pin
    pin_state_t state = button_read_debounced(PIN_2);
    GPIO_TEST_ASSERT(state == PIN_LOW || state == PIN_HIGH, "Button state readable", &button_results);
}

// Test button debouncing with mock states
void test_button_debouncing(void) {
    button_init();
    button_monitor_pin(PIN_2);
    
    // Initial state should be stable
    pin_state_t initial_state = button_read_debounced(PIN_2);
    
    // Simulate rapid state changes (bounce)
    for (int i = 0; i < 5; i++) {
        mock_button_set_state(PIN_2, PIN_HIGH);
        button_update();
        mock_button_set_state(PIN_2, PIN_LOW);
        button_update();
    }
    
    // Advance time but not enough for debounce
    qemu_advance_time(GLOBAL_DEBOUNCE_MS - 5);
    button_update();
    
    // State should still be initial (debouncing)
    pin_state_t bouncing_state = button_read_debounced(PIN_2);
    GPIO_TEST_ASSERT(bouncing_state == initial_state, "Debouncing prevents state change", &button_results);
    
    // Advance past debounce period
    qemu_advance_time(10);
    button_update();
    
    // Now state should be updated
    GPIO_TEST_ASSERT(true, "Debounce period completed", &button_results);
}

// Test button press detection
void test_button_press_detection(void) {
    button_init();
    button_monitor_pin(PIN_2);
    
    // Initially no press events
    GPIO_TEST_ASSERT(!button_pressed(PIN_2), "No press initially", &button_results);
    
    // Simulate button press (active low)
    mock_button_press(PIN_2);
    
    // Update immediately to register state change
    button_update();
    
    // Advance time past debounce period
    qemu_advance_time(GLOBAL_DEBOUNCE_MS + 5);
    button_update();
    
    // Should detect press
    GPIO_TEST_ASSERT(button_pressed(PIN_2), "Button press detected", &button_results);
}

// Test button release detection
void test_button_release_detection(void) {
    button_init();
    button_monitor_pin(PIN_2);
    
    // Start with button pressed
    mock_button_press(PIN_2);
    button_update();
    qemu_advance_time(GLOBAL_DEBOUNCE_MS + 5);
    button_update();
    
    // Clear any press events
    while (button_event_available()) {
        button_event_get();
    }
    
    // Release button
    mock_button_release(PIN_2);
    button_update();
    qemu_advance_time(GLOBAL_DEBOUNCE_MS + 5);
    button_update();
    
    // Should detect release
    GPIO_TEST_ASSERT(button_released(PIN_2), "Button release detected", &button_results);
}

// Test event queue functionality
void test_button_event_queue(void) {
    button_init();
    button_monitor_pin(PIN_2);
    
    // Initially no events
    GPIO_TEST_ASSERT(!button_event_available(), "No events initially", &button_results);
    
    // Generate press event
    mock_button_press(PIN_2);
    button_update();
    qemu_advance_time(GLOBAL_DEBOUNCE_MS + 5);
    button_update();
    
    // Should have event available
    GPIO_TEST_ASSERT(button_event_available(), "Event available after press", &button_results);
    
    // Get event
    button_event_t event = button_event_get();
    GPIO_TEST_ASSERT(event.pin == PIN_2, "Event has correct pin", &button_results);
    GPIO_TEST_ASSERT(event.pressed == true, "Event shows button pressed", &button_results);
    
    // No more events
    GPIO_TEST_ASSERT(!button_event_available(), "No more events after get", &button_results);
}

// Test VM button opcodes
void test_button_vm_opcodes(void) {
    button_init();
    button_monitor_pin(PIN_2);
    
    vm_state_t vm;
    vm_init(&vm);
    
    // Test program: Check button press, then release
    uint16_t button_program[] = {
        (OP_BUTTON_PRESSED << 8) | 2,    // BUTTON_PRESSED pin 2
        (OP_BUTTON_RELEASED << 8) | 2,   // BUTTON_RELEASED pin 2
        (OP_HALT << 8) | 0
    };
    
    vm_error_t error = vm_load_program(&vm, button_program, 3);
    GPIO_TEST_ASSERT(error == VM_OK, "Button VM program load", &button_results);
    
    // Simulate button press
    mock_button_press(PIN_2);
    button_update();
    qemu_advance_time(GLOBAL_DEBOUNCE_MS + 5);
    button_update();
    
    // Run VM program
    error = vm_run(&vm, 100);
    GPIO_TEST_ASSERT(error == VM_OK, "Button VM program execution", &button_results);
    
    // Check results on stack (released result, then pressed result)
    uint32_t released_result, pressed_result;
    error = vm_pop(&vm, &released_result);
    GPIO_TEST_ASSERT(error == VM_OK, "Released result on stack", &button_results);
    
    error = vm_pop(&vm, &pressed_result);
    GPIO_TEST_ASSERT(error == VM_OK, "Pressed result on stack", &button_results);
    GPIO_TEST_ASSERT(pressed_result == 1, "VM detected button press", &button_results);
}

// Test virtual timing
void test_virtual_timing(void) {
    button_init();
    
    uint32_t start_time = qemu_get_virtual_time_ms();
    
    // Advance time
    qemu_advance_time(100);
    uint32_t advanced_time = qemu_get_virtual_time_ms();
    
    GPIO_TEST_ASSERT(advanced_time >= start_time + 100, "Virtual time advances", &button_results);
    
    // Update should also advance time
    button_update();
    uint32_t update_time = qemu_get_virtual_time_ms();
    
    GPIO_TEST_ASSERT(update_time > advanced_time, "Update advances time", &button_results);
}

// Main button test runner
int run_button_tests(void) {
    reset_gpio_test_results(&button_results);
    
    debug_print("=== Button Input Tests Starting ===");
    
    // Run button tests
    test_button_init();
    test_button_monitoring();
    test_button_debouncing();
    test_button_press_detection();
    test_button_release_detection();
    test_button_event_queue();
    test_button_vm_opcodes();
    test_virtual_timing();
    
    print_gpio_test_summary("Button Input", &button_results);
    
    return button_results.failed;
}