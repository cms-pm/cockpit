/**
 * @file io_controller_c_wrapper.cpp
 * @brief C wrapper for IOController to enable C guest application testing
 *
 * Provides C-compatible interface for IOController functionality,
 * enabling guest printf integration testing from C code.
 *
 * @author cms-pm
 * @date 2025-09-19
 * @phase 4.9.1
 */

#include "io_controller.h"
#include <cstddef>
#include <new>

// Global IOController instance for C wrapper
static IOController* g_io_controller = nullptr;

extern "C" {

/**
 * @brief Initialize IOController instance for C wrapper
 * @return true if successful, false otherwise
 */
bool iocontroller_initialize(void) {
    if (g_io_controller != nullptr) {
        return true; // Already initialized
    }

    g_io_controller = new(std::nothrow) IOController();
    if (g_io_controller == nullptr) {
        return false;
    }

    return g_io_controller->initialize_hardware();
}

/**
 * @brief Add string to IOController string table
 * @param str Format string to add
 * @param string_id Output parameter for assigned string ID
 * @return true if successful, false otherwise
 */
bool iocontroller_add_string(const char* str, uint8_t* string_id) {
    if (g_io_controller == nullptr || str == nullptr || string_id == nullptr) {
        return false;
    }

    return g_io_controller->add_string(str, *string_id);
}

/**
 * @brief Call vm_printf through IOController with automatic routing
 * @param string_id ID of format string in string table
 * @param args Array of arguments for printf formatting
 * @param arg_count Number of arguments in args array
 * @return true if successful, false otherwise
 */
bool iocontroller_vm_printf(uint8_t string_id, const int32_t* args, uint8_t arg_count) {
    if (g_io_controller == nullptr) {
        return false;
    }

    return g_io_controller->vm_printf(string_id, args, arg_count);
}

/**
 * @brief Cleanup IOController instance
 */
void iocontroller_cleanup(void) {
    if (g_io_controller != nullptr) {
        g_io_controller->reset_hardware();
        delete g_io_controller;
        g_io_controller = nullptr;
    }
}

/**
 * @brief Check if IOController is initialized
 * @return true if initialized, false otherwise
 */
bool iocontroller_is_initialized(void) {
    return (g_io_controller != nullptr) && g_io_controller->is_hardware_initialized();
}

/**
 * @brief Get current string count in IOController
 * @return Number of strings registered, 0 if not initialized
 */
uint8_t iocontroller_get_string_count(void) {
    if (g_io_controller == nullptr) {
        return 0;
    }

    return g_io_controller->get_string_count();
}

} // extern "C"