# ComponentVM Bootloader System Technical Design

**Version**: 1.0  
**Date**: July 15, 2025  
**Author**: Staff Embedded Systems Architect  
**Target Platform**: STM32G431CB WeAct Studio CoreBoard  
**Status**: Phase 4.5.2 Implementation Design  

---

## 📋 Table of Contents

- [Executive Summary](#executive-summary)
- [System Architecture](#system-architecture)
- [Memory Layout & Banking Strategy](#memory-layout--banking-strategy)
- [Trigger Methods](#trigger-methods)
- [State Machine Design](#state-machine-design)
- [Bytecode Transfer Protocol](#bytecode-transfer-protocol)
- [Verification & Copy Process](#verification--copy-process)
- [Failure Resistance Features](#failure-resistance-features)
- [Key Blind Spots](#key-blind-spots)
- [Implementation Guidelines](#implementation-guidelines)

---

## Executive Summary

The ComponentVM bootloader system implements a robust, application-level bootloader that **complements the STM32 system bootloader** for STM32G431CB microcontrollers. While the STM32 system bootloader handles firmware-level updates via DFU/UART, the ComponentVM bootloader specifically manages **bytecode updates** for the ComponentVM virtual machine system.

### **Bootloader Relationship Architecture**

```
┌─────────────────────────────────────────────────────────────────┐
│                    STM32G431CB System Architecture              │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │  STM32 System   │  │  ComponentVM    │  │  ComponentVM    │ │
│  │  Bootloader     │  │  Bootloader     │  │  Hypervisor     │ │
│  │  (ROM)          │  │  (Flash)        │  │  (Flash)        │ │
│  │                 │  │                 │  │                 │ │
│  │ • DFU Mode      │  │ • Bytecode      │  │ • VM Execution  │ │
│  │ • UART Upload   │  │   Transfer      │  │ • Arduino API   │ │
│  │ • System Reset  │  │ • Dual-Bank     │  │ • Memory Mgmt   │ │
│  │ • Hardware      │  │   Management    │  │ • I/O Control   │ │
│  │   Programming   │  │ • CVM-Specific  │  │ • System Init   │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
│           │                      │                      │       │
│           │                      │                      │       │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │   Firmware      │  │   Bytecode      │  │   Bytecode      │ │
│  │   Updates       │  │   Bank A        │  │   Bank B        │ │
│  │   (Complete     │  │   (Active)      │  │   (Receive)     │ │
│  │   System)       │  │                 │  │                 │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

**Key Distinctions:**
- **STM32 System Bootloader**: Handles complete firmware updates (factory/development)
- **ComponentVM Bootloader**: Handles bytecode updates for user programs (application-level)
- **Complementary Operation**: Both serve different update scenarios without conflict

### **Key Features**
- **Multi-Trigger Architecture**: Hardware button, UART command, and flash flag triggers
- **Dual-Bank Strategy**: Separate receive and active bytecode banks for atomic updates
- **Robust State Machine**: Mealy-Moore hybrid design with comprehensive error handling
- **Transport Abstraction**: UART-first with USB CDC drop-in capability
- **Failure Resistance**: CRC validation, atomic operations, rollback mechanisms

### **System Requirements**
- **Platform**: STM32G431CB (128KB Flash, 32KB RAM)
- **Clock**: 168MHz system clock, 48MHz USB clock
- **Communication**: USART1 (115200 baud), future USB CDC
- **Memory**: 16KB bootloader, 48KB hypervisor, 64KB bytecode storage

---

## System Architecture

### **Component Architecture**

```
┌─────────────────────────────────────────────────────────────────┐
│                     ComponentVM Bootloader                      │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │   Trigger       │  │   Transport     │  │   Protocol      │ │
│  │   Detection     │  │   Layer         │  │   Handler       │ │
│  │                 │  │                 │  │                 │ │
│  │ • Button (PC13) │  │ • UART (USART1) │  │ • Command Parse │ │
│  │ • UART Magic    │  │ • USB CDC       │  │ • CRC Verify    │ │
│  │ • Flash Flag    │  │ • ESP32 Bridge  │  │ • State Machine │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │   Memory        │  │   Flash         │  │   Verification  │ │
│  │   Manager       │  │   Manager       │  │   Engine        │ │
│  │                 │  │                 │  │                 │ │
│  │ • Bank Switch   │  │ • Page Erase    │  │ • CRC32 Check   │ │
│  │ • Buffer Mgmt   │  │ • Program       │  │ • Size Valid    │ │
│  │ • Address Map   │  │ • Verify        │  │ • Magic Valid   │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### **Modular Design Philosophy**

**Transport Abstraction**: Clean separation between trigger detection and data transport
```c
typedef struct {
    HAL_StatusTypeDef (*init)(void);
    HAL_StatusTypeDef (*send)(const uint8_t* data, uint16_t len);
    HAL_StatusTypeDef (*receive)(uint8_t* data, uint16_t max_len, uint16_t* actual_len);
    HAL_StatusTypeDef (*deinit)(void);
} transport_interface_t;

// Transport implementations
extern transport_interface_t uart_transport;
extern transport_interface_t usb_cdc_transport;  // Future
```

---

## Memory Layout & Banking Strategy

### **STM32G431CB Flash Layout (128KB)**

```
┌─────────────────────────────────────────────────────────────────┐
│                     Flash Memory Layout                         │
├─────────────────────────────────────────────────────────────────┤
│ 0x08000000 │ Bootloader (16KB)        │ Pages 0-7    │ PROTECTED │
│ 0x08004000 │ ComponentVM Hypervisor   │ Pages 8-31   │ ACTIVE    │
│            │ (48KB)                   │              │           │
│ 0x08010000 │ Bytecode Bank A (ACTIVE) │ Pages 32-47  │ ACTIVE    │
│            │ (32KB)                   │              │           │
│ 0x08018000 │ Bytecode Bank B (RECEIVE)│ Pages 48-63  │ RECEIVE   │
│            │ (32KB)                   │              │           │
│ 0x08020000 │ End of Flash             │              │           │
└─────────────────────────────────────────────────────────────────┘
```

### **Dual-Bank Strategy**

**Bank Management**:
- **Active Bank**: Currently executing bytecode (Bank A or B)
- **Receive Bank**: Target for new bytecode uploads
- **Atomic Switch**: Banks swap roles after successful verification

**Bank Switching Logic**:
```c
typedef enum {
    BANK_A = 0,
    BANK_B = 1
} bytecode_bank_t;

typedef struct {
    uint32_t magic;           // 0x434F4D50 ("COMP")
    uint32_t version;         // Bytecode version
    uint32_t size;            // Bytecode size in bytes
    uint32_t crc32;          // CRC32 checksum
    uint32_t active_bank;     // BANK_A or BANK_B
    uint32_t reserved[3];     // Future use
} bank_metadata_t;

#define BANK_A_BASE_ADDR    0x08010000
#define BANK_B_BASE_ADDR    0x08018000
#define BANK_SIZE           0x8000      // 32KB per bank
#define METADATA_ADDR       0x0801F800  // Last page for metadata
```

**Bank Selection Algorithm**:
```c
bytecode_bank_t get_active_bank(void) {
    bank_metadata_t* metadata = (bank_metadata_t*)METADATA_ADDR;
    
    if (metadata->magic == 0x434F4D50) {
        return (bytecode_bank_t)metadata->active_bank;
    }
    
    return BANK_A;  // Default to Bank A
}

uint32_t get_bank_base_address(bytecode_bank_t bank) {
    return (bank == BANK_A) ? BANK_A_BASE_ADDR : BANK_B_BASE_ADDR;
}
```

---

## Trigger Methods

### **1. Hardware Button Trigger (PC13)**

**Timing**: Immediate detection during bootloader startup
**Priority**: Highest (checked first)
**Use Case**: Development, debugging, field service

**Implementation**:
```c
bool check_button_trigger(void) {
    // Enable GPIOC clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    
    // Configure PC13 as input with pull-up
    GPIOC->MODER &= ~(3 << 26);      // Clear mode bits
    GPIOC->PUPDR |= (1 << 26);       // Pull-up
    
    // Brief delay for settling
    for (volatile int i = 0; i < 1000; i++);
    
    // Check button state (active low)
    return (GPIOC->IDR & (1 << 13)) == 0;
}
```

### **2. UART Command Sequence**

**Timing**: 5-second window after reset
**Priority**: Medium (checked after button and flash flag)
**Use Case**: Automated deployment, CI/CD

**Magic Sequence**: `"BOOTLOADER_ENTER\n"`
**Response**: `"BOOTLOADER_READY\n"`

**Implementation**:
```c
bool check_uart_trigger(void) {
    static char buffer[32];
    static uint8_t pos = 0;
    
    uint32_t timeout = HAL_GetTick() + 5000;  // 5 second timeout
    
    while (HAL_GetTick() < timeout) {
        if (uart_data_available()) {
            char ch = uart_read_char();
            
            if (ch == '\n') {
                buffer[pos] = '\0';
                if (strcmp(buffer, "BOOTLOADER_ENTER") == 0) {
                    uart_send_string("BOOTLOADER_READY\n");
                    return true;
                }
                pos = 0;
            } else if (pos < sizeof(buffer) - 1) {
                buffer[pos++] = ch;
            }
        }
        
        HAL_Delay(1);  // Small delay to prevent busy-waiting
    }
    
    return false;
}
```

### **3. Flash Flag Trigger**

**Timing**: Immediate detection during bootloader startup
**Priority**: High (checked second, after button)
**Use Case**: Application-initiated updates, OTA

**Flag Structure**:
```c
typedef struct {
    uint32_t magic;        // 0x424F4F54 ("BOOT")
    uint32_t trigger;      // 0x00000001 = enter bootloader
    uint32_t sequence;     // Incremental counter
    uint32_t crc32;       // Data integrity check
} bootloader_flag_t;

#define FLAG_MAGIC_BOOTLOADER 0x424F4F54
#define FLAG_TRIGGER_ENTER    0x00000001
#define FLAG_BASE_ADDR        0x0801F800
```

---

## State Machine Design

### **Hybrid Mealy-Moore State Machine**

The bootloader uses a hybrid state machine combining Moore (output depends on state) and Mealy (output depends on state and input) characteristics for robust operation.

**State Definitions**:
```c
typedef enum {
    STATE_STARTUP,              // Initial state after reset
    STATE_TRIGGER_DETECT,       // Checking for trigger conditions
    STATE_BOOTLOADER_ACTIVE,    // Bootloader mode active
    STATE_TRANSPORT_INIT,       // Initialize transport layer
    STATE_HANDSHAKE,           // Establish communication
    STATE_READY,               // Ready for commands
    STATE_RECEIVE_HEADER,      // Receiving upload header
    STATE_RECEIVE_DATA,        // Receiving bytecode data
    STATE_VERIFY,              // Verifying received data
    STATE_PROGRAM,             // Programming flash memory
    STATE_BANK_SWITCH,         // Switching active bank
    STATE_COMPLETE,            // Upload complete
    STATE_ERROR,               // Error state
    STATE_JUMP_APPLICATION     // Jump to application
} bootloader_state_t;
```

### **State Transition Diagram**

```
                    ┌─────────────┐
                    │   STARTUP   │
                    └─────┬───────┘
                          │
                          ▼
                 ┌─────────────────┐
                 │ TRIGGER_DETECT  │
                 └─────┬───────────┘
                       │
        ┌──────────────┼──────────────┐
        │              │              │
        ▼              ▼              ▼
   ┌─────────┐    ┌─────────┐    ┌─────────┐
   │ BUTTON  │    │  UART   │    │  FLASH  │
   │TRIGGER  │    │TRIGGER  │    │ TRIGGER │
   └─────┬───┘    └─────┬───┘    └─────┬───┘
         │              │              │
         └──────────────┼──────────────┘
                        │
                        ▼
                ┌───────────────┐
                │ BOOTLOADER_   │
                │    ACTIVE     │
                └───────┬───────┘
                        │
                        ▼
                ┌───────────────┐
                │ TRANSPORT_    │
                │    INIT       │
                └───────┬───────┘
                        │
                        ▼
                ┌───────────────┐
                │  HANDSHAKE    │
                └───────┬───────┘
                        │
                        ▼
                ┌───────────────┐
                │     READY     │◄──────────────┐
                └───────┬───────┘               │
                        │                       │
                        ▼                       │
                ┌───────────────┐               │
                │ RECEIVE_      │               │
                │   HEADER      │               │
                └───────┬───────┘               │
                        │                       │
                        ▼                       │
                ┌───────────────┐               │
                │ RECEIVE_      │               │
                │    DATA       │               │
                └───────┬───────┘               │
                        │                       │
                        ▼                       │
                ┌───────────────┐               │
                │    VERIFY     │               │
                └───────┬───────┘               │
                        │                       │
                        ▼                       │
                ┌───────────────┐               │
                │   PROGRAM     │               │
                └───────┬───────┘               │
                        │                       │
                        ▼                       │
                ┌───────────────┐               │
                │  BANK_SWITCH  │               │
                └───────┬───────┘               │
                        │                       │
                        ▼                       │
                ┌───────────────┐               │
                │   COMPLETE    │───────────────┘
                └───────┬───────┘
                        │
                        ▼
                ┌───────────────┐
                │ JUMP_         │
                │ APPLICATION   │
                └───────────────┘
```

### **State Machine Implementation**

```c
typedef struct {
    bootloader_state_t current_state;
    bootloader_state_t next_state;
    uint32_t state_entry_time;
    uint32_t timeout_ms;
    uint8_t retry_count;
    uint8_t max_retries;
} state_machine_t;

void state_machine_run(state_machine_t* sm) {
    switch (sm->current_state) {
        case STATE_STARTUP:
            handle_startup_state(sm);
            break;
            
        case STATE_TRIGGER_DETECT:
            handle_trigger_detect_state(sm);
            break;
            
        case STATE_BOOTLOADER_ACTIVE:
            handle_bootloader_active_state(sm);
            break;
            
        case STATE_TRANSPORT_INIT:
            handle_transport_init_state(sm);
            break;
            
        case STATE_HANDSHAKE:
            handle_handshake_state(sm);
            break;
            
        case STATE_READY:
            handle_ready_state(sm);
            break;
            
        case STATE_RECEIVE_HEADER:
            handle_receive_header_state(sm);
            break;
            
        case STATE_RECEIVE_DATA:
            handle_receive_data_state(sm);
            break;
            
        case STATE_VERIFY:
            handle_verify_state(sm);
            break;
            
        case STATE_PROGRAM:
            handle_program_state(sm);
            break;
            
        case STATE_BANK_SWITCH:
            handle_bank_switch_state(sm);
            break;
            
        case STATE_COMPLETE:
            handle_complete_state(sm);
            break;
            
        case STATE_ERROR:
            handle_error_state(sm);
            break;
            
        case STATE_JUMP_APPLICATION:
            handle_jump_application_state(sm);
            break;
    }
    
    // Check for timeouts
    if (sm->timeout_ms > 0 && 
        (HAL_GetTick() - sm->state_entry_time) > sm->timeout_ms) {
        transition_to_state(sm, STATE_ERROR);
    }
}
```

---

## Bytecode Transfer Protocol

### **Protocol Overview**

The bootloader implements a simple, reliable text-based protocol with binary data support and comprehensive error handling.

**Protocol Features**:
- **Text Commands**: Human-readable command structure
- **Binary Data**: Efficient bytecode transfer
- **CRC Validation**: Data integrity checking
- **Chunked Transfer**: Large file support with progress tracking
- **Error Recovery**: Retry mechanisms and graceful degradation

### **Command Structure**

```
Command Format: "<COMMAND> [parameters]\n"
Response Format: "<STATUS> [data]\n"

Commands:
- SYNC                          → BOOTLOADER_READY v1.0
- UPLOAD_START <size>           → READY_FOR_DATA
- DATA <chunk_size> <data> <crc> → CHUNK_OK | CHUNK_ERROR
- UPLOAD_COMPLETE              → UPLOAD_SUCCESS | UPLOAD_FAILED
- RESET                        → RESETTING
- STATUS                       → STATUS_OK <state>
```

### **Transfer Sequence**

```c
typedef enum {
    PROTOCOL_IDLE,
    PROTOCOL_SYNC,
    PROTOCOL_UPLOAD_START,
    PROTOCOL_DATA_TRANSFER,
    PROTOCOL_UPLOAD_COMPLETE,
    PROTOCOL_ERROR
} protocol_state_t;

typedef struct {
    uint32_t total_size;
    uint32_t bytes_received;
    uint32_t chunk_count;
    uint32_t current_chunk;
    uint32_t crc32_running;
    uint8_t* receive_buffer;
    uint32_t receive_bank_addr;
} transfer_context_t;
```

**Complete Transfer Flow**:
1. **Handshake**: `SYNC` → `BOOTLOADER_READY v1.0`
2. **Upload Start**: `UPLOAD_START 2048` → `READY_FOR_DATA`
3. **Data Chunks**: `DATA 256 <data> <crc>` → `CHUNK_OK`
4. **Completion**: `UPLOAD_COMPLETE` → `UPLOAD_SUCCESS`
5. **Reset**: `RESET` → `RESETTING`

---

## Verification & Copy Process

### **Three-Stage Verification**

**Stage 1: Chunk-Level Verification**
```c
bool verify_chunk(const uint8_t* data, uint16_t size, uint16_t expected_crc) {
    uint16_t calculated_crc = calculate_crc16(data, size);
    return calculated_crc == expected_crc;
}
```

**Stage 2: Complete Bytecode Verification**
```c
bool verify_complete_bytecode(uint32_t bank_addr, uint32_t size) {
    // Verify size constraints
    if (size > BANK_SIZE) return false;
    
    // Calculate CRC32 of entire bytecode
    uint32_t calculated_crc = calculate_crc32((uint8_t*)bank_addr, size);
    
    // Compare with expected CRC from header
    transfer_context_t* ctx = get_transfer_context();
    return calculated_crc == ctx->expected_crc32;
}
```

**Stage 3: Metadata Verification**
```c
bool verify_bytecode_metadata(uint32_t bank_addr) {
    bytecode_header_t* header = (bytecode_header_t*)bank_addr;
    
    // Verify magic number
    if (header->magic != BYTECODE_MAGIC) return false;
    
    // Verify version compatibility
    if (header->version > MAX_SUPPORTED_VERSION) return false;
    
    // Verify size alignment
    if (header->size % 4 != 0) return false;
    
    return true;
}
```

### **Atomic Bank Copy Process**

**Copy Operation**:
```c
HAL_StatusTypeDef copy_to_active_bank(void) {
    bytecode_bank_t current_active = get_active_bank();
    bytecode_bank_t new_active = (current_active == BANK_A) ? BANK_B : BANK_A;
    
    uint32_t source_addr = get_bank_base_address(new_active);
    uint32_t dest_addr = get_bank_base_address(current_active);
    
    // Step 1: Verify source bank integrity
    if (!verify_complete_bytecode(source_addr, get_bytecode_size())) {
        return HAL_ERROR;
    }
    
    // Step 2: Erase destination bank
    HAL_StatusTypeDef status = erase_bank(current_active);
    if (status != HAL_OK) return status;
    
    // Step 3: Copy bytecode
    status = copy_bytecode(source_addr, dest_addr, get_bytecode_size());
    if (status != HAL_OK) return status;
    
    // Step 4: Verify copy integrity
    if (!verify_complete_bytecode(dest_addr, get_bytecode_size())) {
        return HAL_ERROR;
    }
    
    // Step 5: Update metadata (atomic operation)
    status = update_active_bank_metadata(current_active);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}
```

---

## Failure Resistance Features

### **Power-Safe Operations**

**Atomic Bank Switching**:
- Metadata updated only after successful copy
- Previous bank remains valid until new bank verified
- Rollback capability if power fails during update

**Flash Programming Safety**:
```c
typedef struct {
    uint32_t operation_id;
    uint32_t bank_addr;
    uint32_t size;
    uint32_t crc32;
    uint32_t status;       // IN_PROGRESS, COMPLETE, FAILED
} flash_operation_log_t;

#define FLASH_OP_LOG_ADDR  0x0801F700  // Before metadata
```

### **Error Recovery Mechanisms**

**Checksum Validation**:
- CRC16 for individual chunks
- CRC32 for complete bytecode
- Magic number verification
- Size boundary checking

**Communication Recovery**:
```c
typedef struct {
    uint8_t retry_count;
    uint8_t max_retries;
    uint32_t timeout_ms;
    uint32_t last_activity;
} recovery_context_t;

#define MAX_CHUNK_RETRIES    3
#define COMMUNICATION_TIMEOUT 30000  // 30 seconds
```

**Rollback Capability**:
```c
bool rollback_to_previous_bank(void) {
    // Validate current active bank
    bytecode_bank_t current = get_active_bank();
    
    if (!verify_complete_bytecode(get_bank_base_address(current), 
                                 get_bytecode_size())) {
        // Current bank corrupted, switch to other bank
        bytecode_bank_t other = (current == BANK_A) ? BANK_B : BANK_A;
        
        if (verify_complete_bytecode(get_bank_base_address(other), 
                                    get_bytecode_size())) {
            update_active_bank_metadata(other);
            return true;
        }
    }
    
    return false;
}
```

### **Watchdog Integration**

**Bootloader Watchdog**:
```c
void configure_bootloader_watchdog(void) {
    // Configure IWDG for bootloader operations
    // Longer timeout for flash operations
    IWDG->KR = 0xCCCC;     // Start watchdog
    IWDG->KR = 0x5555;     // Enable write access
    IWDG->PR = 0x06;       // Prescaler /256
    IWDG->RLR = 0x0FFF;    // Maximum reload value (~32 seconds)
    IWDG->KR = 0xAAAA;     // Refresh watchdog
}

void refresh_bootloader_watchdog(void) {
    IWDG->KR = 0xAAAA;     // Refresh watchdog
}
```

---

## Key Blind Spots

### **⚠️ Critical Vulnerabilities**

**1. Flash Corruption During Programming**
- **Risk**: Power loss during flash erase/program operations
- **Mitigation**: Dual-bank strategy, operation logging, rollback capability
- **Monitoring**: Flash operation status, power supply monitoring

**2. Communication Timeouts**
- **Risk**: Host tool disconnection during transfer
- **Mitigation**: Robust timeout handling, retry mechanisms
- **Monitoring**: Communication activity, connection status

**3. Metadata Corruption**
- **Risk**: Bank metadata corruption prevents proper bank selection
- **Mitigation**: Redundant metadata storage, CRC validation
- **Monitoring**: Metadata integrity checks on startup

**4. Clock Configuration Drift**
- **Risk**: UART baud rate errors due to clock instability
- **Mitigation**: Robust clock initialization, autobaud detection
- **Monitoring**: Clock source monitoring, communication errors

### **🔍 Monitoring & Diagnostics**

**Bootloader Health Monitoring**:
```c
typedef struct {
    uint32_t boot_count;
    uint32_t successful_updates;
    uint32_t failed_updates;
    uint32_t corruption_events;
    uint32_t rollback_events;
    uint32_t last_error_code;
} bootloader_health_t;

#define HEALTH_DATA_ADDR  0x0801F600  // Before operation log
```

**Diagnostic Commands**:
```c
// Extended protocol for diagnostics
Commands:
- HEALTH_STATUS    → Health statistics
- BANK_STATUS      → Bank information
- MEMORY_TEST      → Flash memory test
- RESET_STATS      → Clear health counters
```

### **🚨 Edge Cases & Failure Modes**

**1. Partial Transfer Failure**
- **Scenario**: Transfer interrupted mid-process
- **Handling**: Restart from beginning, no partial programming
- **Recovery**: Timeout detection, connection reset

**2. Both Banks Corrupted**
- **Scenario**: Both bytecode banks contain invalid data
- **Handling**: Factory reset to default bytecode
- **Recovery**: Emergency programming mode

**3. Bootloader Corruption**
- **Scenario**: Bootloader itself becomes corrupted
- **Handling**: Hardware write protection, external programming
- **Recovery**: ST-Link/SWD recovery, DFU mode

**4. Infinite Bootloader Loop**
- **Scenario**: Bootloader trigger never clears
- **Handling**: Trigger validation, timeout mechanisms
- **Recovery**: Power cycle, hardware reset

---

## Production Reliability Improvements

### **Critical Immediate Improvements (Phase 4.5.2A)**

Based on comprehensive reliability analysis, the following improvements are **production-critical** and must be implemented in Phase 4.5.2A:

#### **1. Hierarchical Error States with Context**

**Problem**: Single `STATE_ERROR` provides no diagnostic capability, making field debugging impossible.

**Implementation**:
```c
// Enhanced error states with context
typedef enum {
    // Operational states
    STATE_STARTUP,
    STATE_TRIGGER_DETECT,
    STATE_BOOTLOADER_ACTIVE,
    STATE_TRANSPORT_INIT,
    STATE_HANDSHAKE,
    STATE_READY,
    STATE_RECEIVE_HEADER,
    STATE_RECEIVE_DATA,
    STATE_VERIFY,
    STATE_PROGRAM,
    STATE_BANK_SWITCH,
    STATE_COMPLETE,
    
    // Context-aware error states
    STATE_ERROR_COMMUNICATION,     // UART timeout, framing errors
    STATE_ERROR_FLASH_OPERATION,   // Flash erase/write failures  
    STATE_ERROR_DATA_CORRUPTION,   // CRC mismatches, invalid data
    STATE_ERROR_RESOURCE_EXHAUSTION, // Memory/buffer issues
    STATE_ERROR_TIMEOUT,           // Generic timeout errors
    STATE_ERROR_HARDWARE_FAULT,    // Hardware-specific failures
    
    // Recovery states
    STATE_RECOVERY_RETRY,
    STATE_RECOVERY_ABORT,
    
    STATE_JUMP_APPLICATION
} bootloader_state_t;

typedef enum {
    ERROR_UART_TIMEOUT,
    ERROR_UART_FRAMING,
    ERROR_UART_OVERRUN,
    ERROR_FLASH_ERASE_FAILED,
    ERROR_FLASH_WRITE_FAILED,
    ERROR_FLASH_VERIFY_FAILED,
    ERROR_CRC_MISMATCH,
    ERROR_BUFFER_OVERFLOW,
    ERROR_INVALID_COMMAND,
    ERROR_SEQUENCE_ERROR,
    ERROR_HARDWARE_FAULT
} bootloader_error_code_t;
```

#### **2. Overflow-Safe Timeout Management**

**Problem**: `HAL_GetTick()` wraps every 49.7 days, causing mysterious timeout failures.

**Implementation**:
```c
typedef struct {
    uint32_t start_tick;
    uint32_t timeout_ms;
    uint32_t warning_ms;
    uint8_t retry_count;
    uint8_t max_retries;
    bool timeout_enabled;
    bool warning_fired;
} timeout_context_t;

// Overflow-safe timeout checking
bool is_timeout_expired(timeout_context_t* ctx) {
    if (!ctx->timeout_enabled) return false;
    
    uint32_t current_tick = HAL_GetTick();
    uint32_t elapsed;
    
    // Handle tick overflow safely
    if (current_tick >= ctx->start_tick) {
        elapsed = current_tick - ctx->start_tick;
    } else {
        elapsed = (UINT32_MAX - ctx->start_tick) + current_tick + 1;
    }
    
    return elapsed >= ctx->timeout_ms;
}
```

#### **3. Resource Cleanup Framework**

**Problem**: No explicit resource management leads to memory leaks, hardware lockups, and flash corruption.

**Implementation**:
```c
typedef struct {
    bool uart_initialized;
    bool flash_unlocked;
    bool dma_active;
    bool interrupts_disabled;
    uint32_t allocated_buffers;
    void (*cleanup_functions[MAX_RESOURCES])(void);
    uint8_t cleanup_count;
} resource_manager_t;

// Resource-safe state transitions
HAL_StatusTypeDef transition_to_state_safe(state_machine_t* sm, 
                                          bootloader_state_t new_state) {
    // Cleanup current state resources
    cleanup_state_resources(sm->current_state);
    
    // Initialize new state resources
    HAL_StatusTypeDef status = initialize_state_resources(new_state);
    if (status != HAL_OK) {
        transition_to_error_state(sm, ERROR_RESOURCE_EXHAUSTION, status);
        return status;
    }
    
    // Perform atomic state transition
    __disable_irq();
    sm->current_state = new_state;
    sm->state_entry_time = HAL_GetTick();
    __enable_irq();
    
    return HAL_OK;
}
```

### **Important Near-Term Improvements (Phase 4.5.2B-C)**

#### **4. Interrupt-Safe State Management**
```c
typedef struct {
    volatile bootloader_state_t current_state;
    volatile bootloader_state_t pending_state;
    volatile bool state_change_pending;
    uint32_t critical_section_depth;
} interrupt_safe_state_machine_t;

void request_state_transition(interrupt_safe_state_machine_t* sm, 
                             bootloader_state_t new_state) {
    __disable_irq();
    sm->pending_state = new_state;
    sm->state_change_pending = true;
    __enable_irq();
}
```

#### **5. Progressive Error Recovery**
```c
typedef enum {
    RECOVERY_STRATEGY_RETRY,
    RECOVERY_STRATEGY_RESET_PERIPHERAL,
    RECOVERY_STRATEGY_RESET_SYSTEM,
    RECOVERY_STRATEGY_ABORT_GRACEFUL
} recovery_strategy_t;

typedef struct {
    bootloader_error_code_t error_type;
    uint8_t retry_count;
    uint8_t max_retries;
    recovery_strategy_t strategy;
    uint32_t backoff_ms;
} error_recovery_context_t;
```

### **Future Improvements (Phase 5)**

#### **6. Watchdog Integration**
```c
typedef struct {
    uint32_t max_execution_time_ms;
    uint32_t watchdog_refresh_interval_ms;
    uint32_t last_watchdog_refresh;
    bool watchdog_enabled;
} watchdog_context_t;
```

#### **7. Power Management Integration**
```c
typedef enum {
    POWER_MODE_ACTIVE,
    POWER_MODE_LOW_POWER,
    POWER_MODE_STANDBY
} power_mode_t;

typedef struct {
    power_mode_t current_mode;
    uint32_t power_budget_mw;
    bool low_power_warning;
    bool critical_power_level;
} power_management_t;
```

#### **8. Comprehensive Telemetry**
```c
typedef struct {
    uint32_t state_entry_count[NUM_STATES];
    uint32_t state_execution_time_ms[NUM_STATES];
    uint32_t state_error_count[NUM_STATES];
    uint32_t transition_count[NUM_STATES][NUM_STATES];
    uint32_t total_error_count;
    bootloader_error_code_t recent_errors[ERROR_HISTORY_SIZE];
    uint8_t error_history_index;
} state_machine_telemetry_t;
```

---

## Implementation Guidelines

### **🎯 Phase 4.5.2A Implementation Priority (Updated)**

**Chunk 4.5.2A: UART Transport Foundation + Critical Reliability Improvements**
1. **Transport Interface**: Abstract transport layer
2. **UART Implementation**: Concrete UART transport
3. **Hierarchical Error States**: Context-aware error handling with diagnostic capability
4. **Overflow-Safe Timeout Management**: Tick-safe timeout handling with wraparound protection
5. **Resource Cleanup Framework**: Explicit resource management on state transitions
6. **Enhanced State Machine**: Core state transitions with reliability improvements
7. **Protocol Foundation**: Handshake and basic commands with error recovery

**Chunk 4.5.2B: Protocol Implementation**
1. **Command Parser**: Complete protocol implementation
2. **Transfer Context**: Chunked transfer management
3. **Error Handling**: Retry and recovery mechanisms
4. **Progress Tracking**: Transfer status monitoring

**Chunk 4.5.2C: Flash Operations**
1. **Dual-Bank Management**: Bank switching logic
2. **Flash Programming**: Erase and program operations
3. **Verification Engine**: Multi-stage verification
4. **Metadata Management**: Bank metadata handling

### **📋 Testing Strategy**

**Unit Testing**:
- State machine transitions
- Protocol command parsing
- CRC calculation verification
- Bank switching logic

**Integration Testing**:
- End-to-end transfer scenarios
- Error injection testing
- Power cycle testing
- Communication timeout testing

**Hardware Validation**:
- Real STM32G431CB hardware testing
- Power supply variation testing
- Temperature stress testing
- Long-term reliability testing

### **🔧 Development Tools**

**Host Tools**:
```bash
# Bootloader client
python tools/bootloader_client.py --port /dev/ttyUSB0 --file bytecode.bin

# Diagnostic tool
python tools/bootloader_diag.py --port /dev/ttyUSB0 --command health_status

# Recovery tool
python tools/bootloader_recovery.py --port /dev/ttyUSB0 --emergency-mode
```

**Debug Support**:
- Semihosting debug output
- LED status indicators
- UART debug messages
- Flash operation logging

---

## Conclusion

The ComponentVM bootloader system provides a robust, failure-resistant platform for embedded firmware updates. The dual-bank architecture, comprehensive error handling, and multiple trigger methods ensure reliable operation in both development and production environments.

**Key Success Factors**:
- **Modular Design**: Clean separation of concerns
- **Failure Resistance**: Multiple layers of protection
- **Diagnostic Capability**: Comprehensive monitoring and recovery
- **Scalable Architecture**: Easy transport layer addition

**Next Steps**:
1. Implement Phase 4.5.2A UART transport foundation
2. Develop host tools for testing and deployment
3. Conduct comprehensive hardware validation
4. Add USB CDC transport as drop-in replacement

This design provides the foundation for a production-ready bootloader system that meets the reliability requirements of embedded systems while maintaining the flexibility needed for development and deployment workflows.

---

*Document Version: 1.0 - Initial technical design*  
*Next Update: After Phase 4.5.2A implementation completion*