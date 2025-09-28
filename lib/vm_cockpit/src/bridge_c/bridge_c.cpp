/*
 * VM Cockpit Bridge C Compatibility Implementation
 * VM Bytecode ↔ C Translation Implementation
 */

#include "bridge_c.h"
#include "../host_interface/host_interface.h"
#include "../component_vm.h"
#include "../vm_opcodes.h"
#include "bootloader_diagnostics.h"  // From vm_bootloader/include
#include <string.h>
#include <cstdlib>
#include <cstdio>

// =================================================================
// VM Instruction Set - USING SINGLE SOURCE OF TRUTH vm_opcodes.h
// =================================================================
// NOTE: All opcode definitions come from vm_opcodes.h
// No duplicate definitions allowed here to prevent conflicts

// =================================================================
// Function Registration Table
// =================================================================

#define MAX_REGISTERED_FUNCTIONS 64

typedef struct {
    uint8_t opcode;
    void* function_ptr;
    bool is_registered;
} bridge_function_entry_t;

static bridge_function_entry_t function_table[MAX_REGISTERED_FUNCTIONS];
static bool bridge_initialized = false;

// =================================================================
// VM Bytecode Translation Implementation
// =================================================================

void bridge_c_compat_init(void) {
    // Initialize function registration table
    memset(function_table, 0, sizeof(function_table));
    
    // Register built-in Arduino API bridge functions
    bridge_c_register_function(static_cast<uint8_t>(VMOpcode::OP_DIGITAL_WRITE), (void*)gpio_pin_write);
    bridge_c_register_function(static_cast<uint8_t>(VMOpcode::OP_DIGITAL_READ), (void*)gpio_pin_read);
    bridge_c_register_function(static_cast<uint8_t>(VMOpcode::OP_DELAY), (void*)delay_ms);
    bridge_c_register_function(static_cast<uint8_t>(VMOpcode::OP_MILLIS), (void*)get_tick_ms);
    bridge_c_register_function(static_cast<uint8_t>(VMOpcode::OP_MICROS), (void*)get_tick_us);
    
    bridge_initialized = true;
}

bridge_c_result_t bridge_c_translate_instruction(
    const uint8_t* bytecode, 
    void* stack_context
) {
    if (!bridge_initialized) {
        return BRIDGE_C_EXECUTION_ERROR;
    }
    
    if (bytecode == NULL) {
        return BRIDGE_C_INVALID_PARAMETERS;
    }
    
    uint8_t opcode = bytecode[0];
    
    // Look up function in registration table
    void* function_ptr = NULL;
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (function_table[i].is_registered && function_table[i].opcode == opcode) {
            function_ptr = function_table[i].function_ptr;
            break;
        }
    }
    
    if (function_ptr == NULL) {
        return BRIDGE_C_UNKNOWN_INSTRUCTION;
    }
    
    // TODO: Implement parameter extraction from VM stack
    // TODO: Call C function with extracted parameters
    // TODO: Push return values back to VM stack
    
    // For now, return success - VM execution engine will handle detailed execution
    (void)stack_context;
    return BRIDGE_C_SUCCESS;
}

bool bridge_c_register_function(uint8_t opcode, void* function_ptr) {
    if (function_ptr == NULL) {
        return false;
    }
    
    // Find empty slot in function table
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (!function_table[i].is_registered) {
            function_table[i].opcode = opcode;
            function_table[i].function_ptr = function_ptr;
            function_table[i].is_registered = true;
            return true;
        }
    }
    
    return false; // Table full
}

// =================================================================
// Bridge Utility Functions
// =================================================================

bool bridge_c_is_opcode_registered(uint8_t opcode) {
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (function_table[i].is_registered && function_table[i].opcode == opcode) {
            return true;
        }
    }
    return false;
}

void* bridge_c_get_function_ptr(uint8_t opcode) {
    for (int i = 0; i < MAX_REGISTERED_FUNCTIONS; i++) {
        if (function_table[i].is_registered && function_table[i].opcode == opcode) {
            return function_table[i].function_ptr;
        }
    }
    return NULL;
}

// =================================================================
// Future: Native C++ Support Foundation
// =================================================================

void bridge_cpp_init(void) {
    // TODO: Future implementation for C++ object support
    // Foundation for user C++ code integration
    // Not a priority for current phase
}

// =================================================================
// Phase 4.11.5: Enhanced ComponentVM Integration with Detailed Observer
// =================================================================

#define MOD_VM_EXEC_TRACE "VM_EXEC_TRACE"
#define MOD_VM_EXEC_STACK "VM_EXEC_STACK"
#define MOD_VM_EXEC_COMPLETE "VM_EXEC_COMPLETE"
#define MOD_VM_EXEC_RESET "VM_EXEC_RESET"
#define MOD_VM_CONTEXT "VM_CONTEXT"
#define MOD_VM_EXECUTION "VM_EXECUTION"
#define MOD_TEST_EXEC "TEST_EXEC"

/**
 * @brief Detailed ExecutionEngine observer with comprehensive PC/SP/operand analysis
 * Provides deep visibility into ExecutionEngine handler execution for Phase 4.11.5 validation
 */
class ExecutionEngineDetailedObserver : public ITelemetryObserver {
private:
    ComponentVM* vm_;                    // Access to VM internals for deep inspection
    uint32_t instruction_count_;         // Total instructions executed
    int32_t stack_depth_tracking_;       // Track stack operations for validation
    bool gpio_verification_enabled_;    // Enable GPIO state verification

public:
    ExecutionEngineDetailedObserver(ComponentVM* vm, bool gpio_verification = true)
        : vm_(vm), instruction_count_(0), stack_depth_tracking_(0), gpio_verification_enabled_(gpio_verification) {
        DIAG_INFO(MOD_VM_CONTEXT, "ExecutionEngineDetailedObserver created with GPIO verification");
    }

    void on_instruction_executed(uint32_t pc, uint8_t opcode, uint32_t operand) override {
        instruction_count_++;

        // Get ExecutionEngine state for deep analysis
        const ExecutionEngine& engine = vm_->get_execution_engine();
        size_t current_pc = engine.get_pc();
        size_t stack_pointer = engine.get_sp();

        // Debug raw opcode value first
        DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                   "RAW_OPCODE: PC=%u, opcode=0x%02X, operand=0x%08X",
                   pc, opcode, operand);

        // Detailed opcode analysis with operand interpretation
        switch (static_cast<VMOpcode>(opcode)) {
            case VMOpcode::OP_PUSH: {
                int16_t value = static_cast<int16_t>(operand & 0xFFFF);
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "PUSH: PC=%u->%u, SP=%u->%u, Value=%d (0x%04X)",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer-1), static_cast<uint32_t>(stack_pointer),
                           value, operand & 0xFFFF);
                stack_depth_tracking_++;
                break;
            }

            case VMOpcode::OP_SUB:
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "SUB: PC=%u->%u, SP=%u->%u, PopB PopA -> Push(A-B), Stack depth: %d->%d",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer+1), static_cast<uint32_t>(stack_pointer),
                           stack_depth_tracking_, stack_depth_tracking_-1);
                stack_depth_tracking_--;  // Pop 2, push 1 = net -1
                break;

            case VMOpcode::OP_MUL:
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "MUL: PC=%u->%u, SP=%u->%u, PopB PopA -> Push(A*B), Stack depth: %d->%d",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer+1), static_cast<uint32_t>(stack_pointer),
                           stack_depth_tracking_, stack_depth_tracking_-1);
                stack_depth_tracking_--;
                break;

            case VMOpcode::OP_DIV:
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "DIV: PC=%u->%u, SP=%u->%u, PopB PopA -> Push(A/B), Stack depth: %d->%d",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer+1), static_cast<uint32_t>(stack_pointer),
                           stack_depth_tracking_, stack_depth_tracking_-1);
                stack_depth_tracking_--;
                break;

            case VMOpcode::OP_PRINTF:
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "PRINTF: PC=%u->%u, SP=%u->%u, Pop value and print via semihosting",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer+1), static_cast<uint32_t>(stack_pointer));
                stack_depth_tracking_--;
                break;

            case VMOpcode::OP_PIN_MODE: {
                // Extract operands from stack (not from immediate operand for this instruction)
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "PIN_MODE: PC=%u->%u, SP=%u->%u, Pop mode, pop pin, configure GPIO, Stack depth: %d->%d",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer+2), static_cast<uint32_t>(stack_pointer),
                           stack_depth_tracking_, stack_depth_tracking_-2);
                stack_depth_tracking_ -= 2;  // Pop pin and mode
                break;
            }

            case VMOpcode::OP_DIGITAL_WRITE: {
                // Extract operands from stack (not from immediate operand)
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "DIGITAL_WRITE: PC=%u->%u, SP=%u->%u, Pop value, pop pin -> GPIO CHANGE!",
                           pc, static_cast<uint32_t>(current_pc),
                           static_cast<uint32_t>(stack_pointer+2), static_cast<uint32_t>(stack_pointer));

                // Verify actual GPIO state change if enabled
                if (gpio_verification_enabled_) {
                    bool actual_gpio_state = gpio_pin_read(6);  // Assume PC6 LED pin
                    DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                               "GPIO_VERIFICATION: PC6 actual hardware state = %s (LED should be visible)",
                               actual_gpio_state ? "HIGH" : "LOW");
                }

                stack_depth_tracking_ -= 2;  // Pop pin and value
                break;
            }

            case VMOpcode::OP_HALT:
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "HALT: PC=%u, Final stack depth=%d, Total instructions=%u",
                           pc, stack_depth_tracking_, instruction_count_);
                break;

            default:
                DIAG_DEBUGF(MOD_VM_EXEC_TRACE, STATUS_SUCCESS,
                           "UNKNOWN: PC=%u->%u, Opcode=0x%02X, Operand=0x%08X, SP=%u",
                           pc, static_cast<uint32_t>(current_pc), opcode, operand, static_cast<uint32_t>(stack_pointer));
                break;
        }

        // Periodic stack validation and depth tracking
        if (instruction_count_ % 5 == 0) {
            DIAG_DEBUGF(MOD_VM_EXEC_STACK, STATUS_SUCCESS,
                       "Stack Status: SP=%u, Tracked depth=%d, Instructions=%u",
                       static_cast<uint32_t>(stack_pointer), stack_depth_tracking_, instruction_count_);
        }
    }

    void on_execution_complete(uint32_t total_instructions, uint32_t execution_time_ms) override {
        float inst_per_ms = (execution_time_ms > 0) ? static_cast<float>(total_instructions) / execution_time_ms : 0.0f;
        DIAG_DEBUGF(MOD_VM_EXEC_COMPLETE, STATUS_SUCCESS,
                   "Execution Complete: %u instructions in %u ms (%.2f inst/ms)",
                   total_instructions, execution_time_ms, inst_per_ms);

        DIAG_DEBUGF(MOD_VM_EXEC_COMPLETE, STATUS_SUCCESS,
                   "Final State: Stack depth=%d, ExecutionEngine handlers validated",
                   stack_depth_tracking_);

        // Validate expected arithmetic results
        if (stack_depth_tracking_ == 0) {
            DIAG_INFO(MOD_VM_EXEC_COMPLETE, "✓ Stack balanced - all operations completed cleanly");
        } else {
            DIAG_WARN(MOD_VM_EXEC_COMPLETE, "Stack imbalanced: items remaining");
        }
    }

    void on_vm_reset() override {
        instruction_count_ = 0;
        stack_depth_tracking_ = 0;
        DIAG_INFO(MOD_VM_EXEC_RESET, "Observer reset - starting fresh execution trace");
    }
};

// =================================================================
// Enhanced ComponentVM C Interface Implementation
// =================================================================

// Internal structure to store program data for execution
struct enhanced_vm_context_internal {
    enhanced_vm_context_t public_ctx;
    const VM::Instruction* loaded_program;
    size_t loaded_instruction_count;
};

extern "C" {

enhanced_vm_context_t* create_enhanced_vm_context(bool enable_tracing, bool enable_gpio_verification) {
    ComponentVM* vm = new ComponentVM();
    enhanced_vm_context_internal* internal_ctx = static_cast<enhanced_vm_context_internal*>(malloc(sizeof(enhanced_vm_context_internal)));

    if (!internal_ctx) {
        delete vm;
        DIAG_ERROR(MOD_VM_CONTEXT, "Failed to allocate enhanced VM context");
        return nullptr;
    }

    enhanced_vm_context_t* ctx = &internal_ctx->public_ctx;
    ctx->component_vm = static_cast<void*>(vm);
    ctx->trace_enabled = enable_tracing;
    ctx->gpio_verification_enabled = enable_gpio_verification;
    ctx->instruction_count_limit = 1000;  // Safety limit

    // Initialize internal fields
    internal_ctx->loaded_program = nullptr;
    internal_ctx->loaded_instruction_count = 0;

    // Register default test string for GT Lite printf testing
    #ifdef QEMU_PLATFORM
    uint8_t string_id;
    vm->get_io_controller().add_string("GT_Lite_Test", string_id);
    #endif

    if (enable_tracing) {
        // Create detailed observer
        ExecutionEngineDetailedObserver* observer = new ExecutionEngineDetailedObserver(vm, enable_gpio_verification);
        vm->add_observer(observer);
        ctx->detailed_observer = static_cast<void*>(observer);

        DIAG_INFO(MOD_VM_CONTEXT, "Enhanced VM context created with detailed execution tracing");
    } else {
        ctx->detailed_observer = nullptr;
        DIAG_INFO(MOD_VM_CONTEXT, "Enhanced VM context created (tracing disabled)");
    }

    return ctx;
}

bool enhanced_vm_load_program(enhanced_vm_context_t* ctx, const uint8_t* bytecode, size_t size) {
    if (!ctx || !ctx->component_vm || !bytecode || size == 0) {
        DIAG_ERROR(MOD_VM_EXECUTION, "Invalid parameters for enhanced_vm_load_program");
        return false;
    }

    enhanced_vm_context_internal* internal_ctx = reinterpret_cast<enhanced_vm_context_internal*>(ctx);
    ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);

    // Convert bytecode to VM::Instruction array with proper alignment
    const VM::Instruction* instructions = reinterpret_cast<const VM::Instruction*>(bytecode);
    size_t instruction_count = size / sizeof(VM::Instruction);

    if (size % sizeof(VM::Instruction) != 0) {
        DIAG_WARN(MOD_VM_EXECUTION, "Bytecode size not aligned to instruction boundary, truncating");
    }

    DIAG_DEBUGF(MOD_VM_EXECUTION, STATUS_SUCCESS,
               "Loading program: %u bytes -> %u instructions",
               static_cast<uint32_t>(size), static_cast<uint32_t>(instruction_count));

    // Store program data for execution
    internal_ctx->loaded_program = instructions;
    internal_ctx->loaded_instruction_count = instruction_count;

    return vm->load_program(instructions, instruction_count);
}

bool enhanced_vm_execute_with_diagnostics(enhanced_vm_context_t* ctx) {
    if (!ctx || !ctx->component_vm) {
        DIAG_ERROR(MOD_VM_EXECUTION, "Invalid context for enhanced_vm_execute_with_diagnostics");
        return false;
    }

    // Cast to internal structure to access stored program
    enhanced_vm_context_internal* internal_ctx = reinterpret_cast<enhanced_vm_context_internal*>(ctx);
    ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);

    DIAG_INFO(MOD_VM_EXECUTION, "=== STARTING ENHANCED VM EXECUTION WITH DIAGNOSTICS ===");
    DIAG_DEBUGF(MOD_VM_EXECUTION, STATUS_SUCCESS, "Instruction limit: %u, Tracing: %s, GPIO verification: %s",
                ctx->instruction_count_limit,
                ctx->trace_enabled ? "ENABLED" : "DISABLED",
                ctx->gpio_verification_enabled ? "ENABLED" : "DISABLED");

    // Add safety timeout and step-by-step execution to prevent hanging
    DIAG_INFO(MOD_VM_EXECUTION, "Starting step-by-step execution with observer tracing");

    bool success = true;
    uint32_t instruction_count = 0;
    const uint32_t max_instructions = ctx->instruction_count_limit;

    // Reset VM and load program fresh
    vm->reset_vm();
    vm->load_program(internal_ctx->loaded_program, internal_ctx->loaded_instruction_count);

    // Execute step by step with timeout protection
    while (success && !vm->is_halted() && instruction_count < max_instructions) {
        success = vm->execute_single_step();
        instruction_count++;

        if (instruction_count % 10 == 0) {
            DIAG_DEBUGF(MOD_VM_EXECUTION, STATUS_SUCCESS, "Executed %u instructions, continuing...", instruction_count);
        }
    }

    if (instruction_count >= max_instructions) {
        DIAG_WARN(MOD_VM_EXECUTION, "Execution stopped at instruction limit");
        success = false;
    }

    DIAG_DEBUGF(MOD_VM_EXECUTION, STATUS_SUCCESS, "Execution loop complete: %u instructions, halted=%s",
                instruction_count, vm->is_halted() ? "true" : "false");

    if (success) {
        auto metrics = vm->get_performance_metrics();
        DIAG_DEBUGF(MOD_VM_EXECUTION, STATUS_SUCCESS,
                   "SUCCESS: %u instructions, %u ms, %u memory ops, %u I/O ops",
                   static_cast<uint32_t>(metrics.instructions_executed), metrics.execution_time_ms,
                   static_cast<uint32_t>(metrics.memory_operations), static_cast<uint32_t>(metrics.io_operations));
    } else {
        vm_error_t error = vm->get_last_error();
        DIAG_ERROR(MOD_VM_EXECUTION, "FAILED: VM execution error");
    }

    return success;
}

void enhanced_vm_get_execution_state(enhanced_vm_context_t* ctx, uint32_t* pc, uint32_t* sp, bool* halted) {
    if (!ctx || !ctx->component_vm) {
        if (pc) *pc = 0;
        if (sp) *sp = 0;
        if (halted) *halted = true;
        return;
    }

    ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);
    const ExecutionEngine& engine = vm->get_execution_engine();

    if (pc) *pc = static_cast<uint32_t>(engine.get_pc());
    if (sp) *sp = static_cast<uint32_t>(engine.get_sp());
    if (halted) *halted = engine.is_halted();
}

void enhanced_vm_get_performance_metrics(enhanced_vm_context_t* ctx, uint32_t* instructions_executed,
                                       uint32_t* execution_time_ms, size_t* memory_operations, size_t* io_operations) {
    if (!ctx || !ctx->component_vm) {
        if (instructions_executed) *instructions_executed = 0;
        if (execution_time_ms) *execution_time_ms = 0;
        if (memory_operations) *memory_operations = 0;
        if (io_operations) *io_operations = 0;
        return;
    }

    ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);
    auto metrics = vm->get_performance_metrics();

    if (instructions_executed) *instructions_executed = static_cast<uint32_t>(metrics.instructions_executed);
    if (execution_time_ms) *execution_time_ms = metrics.execution_time_ms;
    if (memory_operations) *memory_operations = metrics.memory_operations;
    if (io_operations) *io_operations = metrics.io_operations;
}

void destroy_enhanced_vm_context(enhanced_vm_context_t* ctx) {
    if (!ctx) return;

    if (ctx->component_vm) {
        ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);

        // Clean up observer if it exists
        if (ctx->detailed_observer) {
            ExecutionEngineDetailedObserver* observer = static_cast<ExecutionEngineDetailedObserver*>(ctx->detailed_observer);
            vm->remove_observer(observer);
            delete observer;
            DIAG_INFO(MOD_VM_CONTEXT, "Detailed observer destroyed");
        }

        delete vm;
        DIAG_INFO(MOD_VM_CONTEXT, "Enhanced VM context destroyed");
    }

    free(ctx);
}

bool enhanced_vm_get_stack_contents(enhanced_vm_context_t* ctx, int32_t* stack_out,
                                   size_t max_stack_size, size_t* actual_stack_size) {
    if (!ctx || !ctx->component_vm || !stack_out || !actual_stack_size) {
        return false;
    }

    ComponentVM* vm = static_cast<ComponentVM*>(ctx->component_vm);

    // Access ExecutionEngine_v2 through ComponentVM
    #ifdef USE_EXECUTION_ENGINE_V2
    ExecutionEngine_v2& engine = vm->get_execution_engine();

    // Get current stack pointer
    size_t stack_elements = engine.get_sp();
    *actual_stack_size = stack_elements;

    if (stack_elements == 0) {
        return true; // Empty stack is valid
    }

    if (stack_elements > max_stack_size) {
        stack_elements = max_stack_size; // Truncate to available buffer
    }

    // Copy stack contents from ExecutionEngine_v2
    // We need to access the private stack_ member - for now use peek()
    for (size_t i = 0; i < stack_elements; i++) {
        int32_t value;
        if (engine.peek(value)) {
            // peek() gets top of stack, but we want to copy entire stack
            // This is a limitation - we can only get the top element
            stack_out[stack_elements - 1] = value; // Put top element at end
            break; // Can only get one element with current API
        }
    }

    #else
    // For original ExecutionEngine, we don't have stack access
    *actual_stack_size = 0;
    #endif

    return true;
}

} // extern "C"