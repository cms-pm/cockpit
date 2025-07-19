/*
 * VM Blackbox Observer Implementation
 * Phase 4.3.2A: Simple bridge between ComponentVM and vm_blackbox
 * 
 * Provides minimal bridge to forward generic ComponentVM execution events
 * to vm_blackbox telemetry system without coupling ComponentVM to blackbox details.
 */

#pragma once

#include "../component_vm.h"
#include "vm_blackbox.h"
#include <cstdint>

class BlackboxObserver : public ITelemetryObserver {
private:
    vm_blackbox_t* blackbox_;
    bool blackbox_owned_;  // Whether this observer owns the blackbox instance
    
public:
    // Constructor that creates its own blackbox instance
    BlackboxObserver();
    
    // Constructor that uses an existing blackbox instance
    explicit BlackboxObserver(vm_blackbox_t* existing_blackbox);
    
    // Destructor
    ~BlackboxObserver();
    
    // ITelemetryObserver interface implementation
    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override;
    void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) override;
    void on_vm_reset() override;
    
    // Access to underlying blackbox for direct inspection
    vm_blackbox_t* get_blackbox() const { return blackbox_; }
    
    // Utility methods for testing
    bool is_blackbox_valid() const { return blackbox_ != nullptr; }
    
    // Disable copy/move to avoid blackbox ownership issues
    BlackboxObserver(const BlackboxObserver&) = delete;
    BlackboxObserver& operator=(const BlackboxObserver&) = delete;
    BlackboxObserver(BlackboxObserver&&) = delete;
    BlackboxObserver& operator=(BlackboxObserver&&) = delete;
};