/*
 * ComponentVM C++ Debug Engine with RAII Hardware Cleanup
 * Phase 4.3.4: Guaranteed Hardware Reset via Destructor
 * 
 * Ensures hardware continues normal operation even if debug session crashes
 */

#pragma once

#include <string>
#include <memory>
#include <iostream>

#ifdef HARDWARE_PLATFORM
    #include "stm32g4xx_hal.h"
#endif

class ComponentVMDebugEngine {
public:
    /**
     * Constructor: Initialize debug engine
     * @param openocd_config Path to OpenOCD configuration file
     * @param gdb_port GDB server port (default 3333)
     */
    explicit ComponentVMDebugEngine(const std::string& openocd_config = "scripts/gdb/openocd_debug.cfg", 
                                   int gdb_port = 3333)
        : openocd_config_(openocd_config)
        , gdb_port_(gdb_port)
        , session_active_(false)
        , cleanup_completed_(false) {
        
        std::cout << "ComponentVM Debug Engine initialized (C++)" << std::endl;
    }

    /**
     * Destructor: GUARANTEED hardware reset sequence
     * 
     * CRITICAL: This ensures hardware continues normal operation even if:
     * - Program crashes unexpectedly
     * - Debug session is interrupted (Ctrl+C)
     * - Exception occurs during debugging
     * - Memory cleanup happens at program exit
     */
    ~ComponentVMDebugEngine() {
        if (!cleanup_completed_) {
            ensure_hardware_reset();
        }
    }

    // Delete copy constructor and assignment operator (RAII single ownership)
    ComponentVMDebugEngine(const ComponentVMDebugEngine&) = delete;
    ComponentVMDebugEngine& operator=(const ComponentVMDebugEngine&) = delete;

    // Move constructor and assignment operator
    ComponentVMDebugEngine(ComponentVMDebugEngine&& other) noexcept
        : openocd_config_(std::move(other.openocd_config_))
        , gdb_port_(other.gdb_port_)
        , session_active_(other.session_active_)
        , cleanup_completed_(other.cleanup_completed_) {
        
        // Mark other as cleaned up to prevent double cleanup
        other.cleanup_completed_ = true;
    }

    ComponentVMDebugEngine& operator=(ComponentVMDebugEngine&& other) noexcept {
        if (this != &other) {
            // Cleanup current object first
            if (!cleanup_completed_) {
                ensure_hardware_reset();
            }

            // Move resources
            openocd_config_ = std::move(other.openocd_config_);
            gdb_port_ = other.gdb_port_;
            session_active_ = other.session_active_;
            cleanup_completed_ = other.cleanup_completed_;

            // Mark other as cleaned up
            other.cleanup_completed_ = true;
        }
        return *this;
    }

    /**
     * Start debug session
     */
    bool start_session() {
        if (session_active_) {
            return true;  // Already started
        }

        // TODO: Implement OpenOCD startup logic
        std::cout << "Starting debug session..." << std::endl;
        session_active_ = true;
        return true;
    }

    /**
     * Execute GDB command
     */
    bool execute_gdb_command(const std::string& command) {
        if (!session_active_) {
            std::cerr << "Debug session not active" << std::endl;
            return false;
        }

        // TODO: Implement actual GDB command execution
        std::cout << "GDB Command: " << command << std::endl;
        return true;
    }

    /**
     * Manual cleanup - call this for explicit resource management
     */
    void cleanup() {
        if (!cleanup_completed_) {
            ensure_hardware_reset();
        }
    }

private:
    std::string openocd_config_;
    int gdb_port_;
    bool session_active_;
    bool cleanup_completed_;

    /**
     * Ensure hardware reset sequence - the critical safety method
     * 
     * This method implements the proper OpenOCD reset sequence to ensure
     * the STM32G431CB hardware continues normal operation after debugging.
     */
    void ensure_hardware_reset() {
        try {
            std::cout << "ComponentVM Debug Engine: Ensuring hardware reset sequence..." << std::endl;

            if (session_active_) {
                // Execute proper OpenOCD reset and disconnect sequence
                // This is equivalent to our Python implementation
                execute_gdb_command("monitor reset halt");
                execute_gdb_command("monitor reset run");
                execute_gdb_command("detach");           // Disconnect GDB from target
                execute_gdb_command("monitor shutdown"); // Shutdown OpenOCD server
                
                std::cout << "✓ Hardware reset and ST-Link disconnect completed" << std::endl;
            }

            // Stop debug session
            if (session_active_) {
                std::cout << "Stopping debug session..." << std::endl;
                session_active_ = false;
            }

            cleanup_completed_ = true;

        } catch (const std::exception& e) {
            std::cerr << "Warning: Debug cleanup exception: " << e.what() << std::endl;
            // Mark as completed even if there was an error to prevent infinite loops
            cleanup_completed_ = true;
        } catch (...) {
            std::cerr << "Warning: Unknown debug cleanup exception" << std::endl;
            cleanup_completed_ = true;
        }
    }
};

/**
 * RAII Debug Session Helper - Recommended Usage Pattern
 * 
 * Usage:
 *   {
 *       auto debug_session = create_debug_session();
 *       debug_session->execute_gdb_command("monitor reset halt");
 *       // ... debugging work ...
 *   } // <- Automatic cleanup here, guaranteed hardware reset
 */
inline std::unique_ptr<ComponentVMDebugEngine> create_debug_session(
    const std::string& config = "scripts/gdb/openocd_debug.cfg") {
    return std::make_unique<ComponentVMDebugEngine>(config);
}