/*
 * Embedded Hypervisor MVP - VM Core
 * Phase 1, Chunk 1.2: Stack-based Virtual Machine
 */

#ifndef VM_CORE_H
#define VM_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// VM Configuration
#define VM_MEMORY_SIZE      0x2000  // 8KB total VM memory
#define VM_STACK_SIZE       0x1000  // 4KB stack
#define VM_HEAP_SIZE        0x1000  // 4KB heap
#define VM_STACK_BASE       0x20000000
#define VM_HEAP_BASE        (VM_STACK_BASE + VM_STACK_SIZE)

// Bytecode instruction format: 16-bit (8-bit opcode + 8-bit immediate)
typedef struct {
    uint8_t opcode;
    uint8_t immediate;
} vm_instruction_t;

// Basic VM opcodes
typedef enum {
    OP_NOP    = 0x00,
    OP_PUSH   = 0x01,
    OP_POP    = 0x02,
    OP_ADD    = 0x03,
    OP_SUB    = 0x04,
    OP_MUL    = 0x05,
    OP_DIV    = 0x06,
    OP_MOD    = 0x07,
    OP_CALL   = 0x08,
    OP_RET    = 0x09,
    // Arduino function opcodes (0x10-0x1F reserved)
    OP_DIGITAL_WRITE = 0x10,
    OP_DIGITAL_READ  = 0x11,
    OP_ANALOG_WRITE  = 0x12,
    OP_ANALOG_READ   = 0x13,
    OP_DELAY         = 0x14,
    OP_BUTTON_PRESSED = 0x15,    // Check if button pressed since last check
    OP_BUTTON_RELEASED = 0x16,   // Check if button released since last check
    OP_PIN_MODE      = 0x17,     // Set pin mode (INPUT, OUTPUT, INPUT_PULLUP)
    OP_PRINTF        = 0x18,     // Print formatted string to semihosting
    OP_MILLIS        = 0x19,     // Get milliseconds since boot
    OP_MICROS        = 0x1A,     // Get microseconds since boot
    // Comparison operations (0x20-0x2F reserved)  
    // Unsigned comparisons (0x20-0x25)
    OP_EQ = 0x20,  OP_NE = 0x21, // Equal, Not Equal
    OP_LT = 0x22,  OP_GT = 0x23, // Less Than, Greater Than
    OP_LE = 0x24,  OP_GE = 0x25, // Less/Greater or Equal
    // Signed comparisons (0x26-0x2B)  
    OP_EQ_S = 0x26, OP_NE_S = 0x27, // Equal, Not Equal (signed)
    OP_LT_S = 0x28, OP_GT_S = 0x29, // Less Than, Greater Than (signed)
    OP_LE_S = 0x2A, OP_GE_S = 0x2B, // Less/Greater or Equal (signed)
    // Control flow operations (0x30-0x3F reserved)
    OP_JMP = 0x30,       // Unconditional jump by signed immediate offset
    OP_JMP_TRUE = 0x31,  // Jump if FLAG_ZERO == 1 (comparison result true)
    OP_JMP_FALSE = 0x32, // Jump if FLAG_ZERO == 0 (comparison result false)
    // Logical operations (0x40-0x4F reserved)
    OP_AND = 0x40,       // Logical AND (&&) - implements short-circuit evaluation
    OP_OR = 0x41,        // Logical OR (||) - implements short-circuit evaluation
    OP_NOT = 0x42,       // Logical NOT (!)
    OP_HALT          = 0xFF
} vm_opcode_t;

// VM state structure
typedef struct {
    uint32_t stack_memory[VM_STACK_SIZE / sizeof(uint32_t)];  // Actual stack memory
    uint32_t heap_memory[VM_HEAP_SIZE / sizeof(uint32_t)];    // Actual heap memory
    uint32_t *stack;           // Stack pointer
    uint32_t *stack_base;      // Base of stack
    uint32_t *stack_top;       // Top of stack (max address)
    uint32_t *heap;            // Heap pointer
    uint16_t *program;         // Program counter
    uint16_t *program_base;    // Base of program memory
    uint32_t program_size;     // Program size in instructions
    bool     running;          // VM execution state
    uint32_t cycle_count;      // Instruction cycle counter
    uint8_t  flags;            // Multi-bit flags register
} vm_state_t;

// VM error codes
typedef enum {
    VM_OK = 0,
    VM_ERROR_STACK_OVERFLOW,
    VM_ERROR_STACK_UNDERFLOW,
    VM_ERROR_INVALID_OPCODE,
    VM_ERROR_INVALID_ADDRESS,
    VM_ERROR_DIVISION_BY_ZERO,
    VM_ERROR_INVALID_JUMP,
    VM_ERROR_STACK_CORRUPTION,
    VM_ERROR_HEAP_CORRUPTION,
    VM_ERROR_MEMORY_PROTECTION
} vm_error_t;

// Core VM functions
vm_error_t vm_init(vm_state_t *vm);
vm_error_t vm_load_program(vm_state_t *vm, uint16_t *program, uint32_t size);
vm_error_t vm_execute_instruction(vm_state_t *vm);
vm_error_t vm_run(vm_state_t *vm, uint32_t max_cycles);

// Stack operations
vm_error_t vm_push(vm_state_t *vm, uint32_t value);
vm_error_t vm_pop(vm_state_t *vm, uint32_t *value);
bool vm_stack_bounds_check(vm_state_t *vm, uint32_t *address);

// Debugging
void vm_dump_state(vm_state_t *vm);

// Printf implementation
void vm_printf(uint32_t format_addr, uint32_t *args, uint32_t arg_count);

// Flag definitions
#define FLAG_ZERO 0x01  // Comparison result (1=true, 0=false)

// Memory protection constants
#define STACK_CANARY_MAGIC 0xDEADBEEF
#define HEAP_GUARD_MAGIC   0xFEEDFACE

// Memory protection functions
vm_error_t vm_check_stack_canaries(vm_state_t *vm);
vm_error_t vm_check_heap_guards(vm_state_t *vm);
vm_error_t vm_init_memory_protection(vm_state_t *vm);

#endif // VM_CORE_H