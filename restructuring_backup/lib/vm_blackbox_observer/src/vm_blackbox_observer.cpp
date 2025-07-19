/*
 * VM Blackbox Observer Implementation  
 * Phase 4.3.2A: Simple bridge between ComponentVM and vm_blackbox
 */

#include "../include/vm_blackbox_observer.h"

// Semihosting for debug output
extern "C" {
    #include "../../semihosting/semihosting.h"
}

BlackboxObserver::BlackboxObserver() 
    : blackbox_(nullptr), blackbox_owned_(true)
{
    // Create our own blackbox instance
    blackbox_ = vm_blackbox_create();
    if (!blackbox_) {
        debug_print("ERROR: Failed to create blackbox instance in BlackboxObserver");
    } else {
        debug_print("BlackboxObserver created with new blackbox instance");
    }
}

BlackboxObserver::BlackboxObserver(vm_blackbox_t* existing_blackbox)
    : blackbox_(existing_blackbox), blackbox_owned_(false)
{
    if (!blackbox_) {
        debug_print("WARNING: BlackboxObserver created with null blackbox");
    } else {
        debug_print("BlackboxObserver created with existing blackbox instance");
    }
}

BlackboxObserver::~BlackboxObserver()
{
    if (blackbox_ && blackbox_owned_) {
        vm_blackbox_destroy(blackbox_);
        debug_print("BlackboxObserver destroyed blackbox instance");
    }
    blackbox_ = nullptr;
}

void BlackboxObserver::on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand)
{
    if (!blackbox_) {
        return;  // Fail silently if no blackbox
    }
    
    // Update blackbox with execution information
    // Note: vm_blackbox_update_execution expects (pc, instruction_count, last_opcode)
    // We use operand as a generic data field - tests can interpret as needed
    vm_blackbox_update_execution(blackbox_, pc, static_cast<uint32_t>(operand), static_cast<uint32_t>(opcode));
}

void BlackboxObserver::on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms)
{
    if (!blackbox_) {
        return;  // Fail silently if no blackbox
    }
    
    // Update blackbox with final execution metrics
    // Use a final update to capture completion state
    vm_blackbox_update_execution(blackbox_, 0xFFFFFFFF, total_instructions, execution_time_ms);
    
    debug_print("Execution complete - updated blackbox with final metrics");
    debug_print_dec("Total instructions", total_instructions);
    debug_print_dec("Execution time (ms)", execution_time_ms);
}

void BlackboxObserver::on_vm_reset()
{
    if (!blackbox_) {
        return;  // Fail silently if no blackbox
    }
    
    // Reset blackbox telemetry state
    // Note: vm_blackbox doesn't have a specific reset function,
    // so we update with reset markers
    vm_blackbox_update_execution(blackbox_, 0x00000000, 0, 0);
    
    debug_print("VM reset - blackbox telemetry reset");
}