# CockpitVM Modular Diagnostics Framework
## Technical Specification v1.0 - Phase 4.6.3

### Executive Summary

The CockpitVM Modular Diagnostics Framework represents the **spiritual successor to flow_log**, transforming Oracle protocol debugging from tactical guesswork to strategic precision instrumentation. This system provides comprehensive, timestamped, structured logging via USART2 while maintaining sacred separation from Oracle protocol communication on USART1.

**Core Philosophy**: Hardware-first reliability with surgical debugging precision, designed for immediate utility and future RTOS expansion.

---

## 1. Architecture Overview

### 1.1 System Boundaries and Separation of Concerns

```
┌─────────────────────────────────────────────────────┐
│                  CockpitVM System                   │
├─────────────────────┬───────────────────────────────┤
│   Oracle Protocol  │      Diagnostics System      │
│     (USART1)       │        (USART2)              │
│   PA9/PA10@115200  │      PA2/PA3@115200          │
│                    │                              │
│   • Binary framing │   • Timestamped logging      │
│   • Protobuf msgs  │   • Status code tracking     │
│   • CRC validation │   • Flow step monitoring     │
│   • SACRED BOUNDARY│   • Binary hex dumps         │
└─────────────────────┴───────────────────────────────┘
```

**Critical Design Principle**: Absolute separation between Oracle protocol (USART1) and diagnostics (USART2). Zero interference guaranteed.

### 1.2 Layered Architecture

```
┌───────────────────────────────────────────────────────┐
│ Application Layer (main.c, protocol handlers)        │
├───────────────────────────────────────────────────────┤
│ DIAG Macro Interface (file/line/function automatic)  │
├───────────────────────────────────────────────────────┤
│ Diagnostics Framework (bootloader_diagnostics.h/c)   │
├───────────────────────────────────────────────────────┤
│ Host Interface (debug_uart_* functions)              │
├───────────────────────────────────────────────────────┤
│ Platform Layer (stm32g4_debug_uart_*)                │
├───────────────────────────────────────────────────────┤
│ STM32 HAL (USART2 PA2/PA3 configuration)             │
└───────────────────────────────────────────────────────┘
```

---

## 2. Component Specifications

### 2.1 Diagnostics Framework Core (`bootloader_diagnostics.h`)

#### 2.1.1 Log Levels
```c
typedef enum {
    LOG_LEVEL_ERROR = 0,    // Critical errors requiring immediate attention
    LOG_LEVEL_WARN  = 1,    // Warnings indicating potential issues  
    LOG_LEVEL_INFO  = 2,    // General information and flow tracking
    LOG_LEVEL_DEBUG = 3,    // Debug information for development
    LOG_LEVEL_TRACE = 4     // Detailed tracing for deep debugging
} log_level_t;
```

#### 2.1.2 Status Codes (Bootloader-Specific)
```c
typedef enum {
    STATUS_SUCCESS = 0,      // Operation completed successfully
    STATUS_ERROR_GENERAL = 1,// General error condition
    STATUS_ERROR_NANOPB = 2, // nanopb encode/decode failure
    STATUS_ERROR_FRAME = 3,  // Frame parsing/validation error
    STATUS_ERROR_PROTOCOL = 4,// Protocol state machine error
    STATUS_ERROR_FLASH = 5,  // Flash programming error
    STATUS_ERROR_MEMORY = 6, // Memory allocation/bounds error
    STATUS_ERROR_TIMEOUT = 7,// Timeout condition
    STATUS_ERROR_CRC = 8,    // CRC validation failure
    STATUS_ERROR_STATE = 9   // Invalid state transition
} status_code_t;
```

#### 2.1.3 Component Identifiers
```c
#define DIAG_COMPONENT_PROTOCOL_ENGINE      0
#define DIAG_COMPONENT_NANOPB_DECODE        1
#define DIAG_COMPONENT_NANOPB_ENCODE        2
#define DIAG_COMPONENT_MESSAGE_HANDLER      3
#define DIAG_COMPONENT_FRAME_PARSER         4
#define DIAG_COMPONENT_FLASH_OPERATIONS     5
#define DIAG_COMPONENT_MAIN_SYSTEM          6
```

### 2.2 Enhanced DIAG Macro Interface

#### 2.2.1 Core Logging Macros
```c
// Automatic file/line/function context capture
#define DIAG_ERROR(comp, status, fmt, ...)   \
    bootloader_diag_log_full(LOG_LEVEL_ERROR, comp, __FILE__, __LINE__, status, fmt, ##__VA_ARGS__)

#define DIAG_INFO(comp, status, fmt, ...)    \
    bootloader_diag_log_full(LOG_LEVEL_INFO, comp, __FILE__, __LINE__, status, fmt, ##__VA_ARGS__)

#define DIAG_DEBUG(comp, status, fmt, ...)   \
    bootloader_diag_log_full(LOG_LEVEL_DEBUG, comp, __FILE__, __LINE__, status, fmt, ##__VA_ARGS__)
```

#### 2.2.2 Specialized Logging Functions
```c
// Flow step tracking (spiritual successor to protocol_flow_log_step)
#define DIAG_FLOW(step, desc, status) \
    bootloader_diag_flow_step(step, desc, status)

// Binary data inspection  
#define DIAG_BUFFER(level, comp, name, data, len) \
    bootloader_diag_hex_dump(name, data, len)
```

#### 2.2.3 Output Format Specification
```
[timestamp] [level] [module] [file:line] [status] message
[00001250] [INFO ] [FRAME] [frame_parser.c:45] [SUCCESS] Step A: Frame start detected
[00001251] [DEBUG] [PROTO] [protocol.c:67] [SUCCESS] Length validation: 32 bytes
[00001252] [ERROR] [PROTO] [protocol.c:123] [ERR_CRC ] CRC mismatch: expected=0xABCD, got=0x1234
```

### 2.3 Flow Logging Enhancement (flow_log Spiritual Successor)

#### 2.3.1 A-J Protocol Step Tracking
Enhanced version of original flow_log with comprehensive context:

```c
// Original flow_log approach
void protocol_flow_log_step(char step); // Basic step tracking

// Enhanced diagnostics approach  
DIAG_FLOW('A', "Frame start detection", STATUS_SUCCESS);
DIAG_FLOW('B', "Length validation complete", STATUS_SUCCESS);  
DIAG_FLOW('C', "Payload extraction", STATUS_SUCCESS);
DIAG_FLOW('D', "CRC validation failed", STATUS_ERROR_CRC);
```

#### 2.3.2 Protocol Flow Debug Buffer
```c
typedef struct {
    char flow_steps[PROTOCOL_FLOW_BUFFER_SIZE];
    uint32_t step_timestamps[PROTOCOL_FLOW_BUFFER_SIZE];
    uint8_t step_count;
    bool flow_complete;
    uint32_t flow_start_time;
} protocol_flow_debug_t;
```

---

## 3. Implementation Roadmap

### 3.1 Phase 1: Global nanpb→nanopb Correction
**Objective**: Fix naming inconsistency across entire codebase

**Scope**: All files, comments, variables, functions, file names
- Replace "nanpb" with "nanopb" in all contexts
- Update include statements, function calls, comments
- Ensure Oracle protocol references use correct naming

**Success Criteria**: 
- Zero instances of "nanpb" remain in codebase
- All nanopb references use consistent naming
- Compilation successful with corrected naming

### 3.2 Phase 2: Diagnostics Framework Activation
**Objective**: Replace stub system with full implementation

**Implementation Steps**:
1. Remove `bootloader_diagnostics_stub.h` inclusion
2. Enable `bootloader_diagnostics.h` inclusion in `bootloader_protocol.h` 
3. Initialize diagnostics framework early in `main.c`
4. Verify USART2 output routing

**Success Criteria**:
- `bootloader_diag_init()` called before any protocol activity
- USART2 receives structured diagnostic output
- No compilation conflicts with existing DIAG macros

### 3.3 Phase 3: DIAG Macro Enhancement Revolution  
**Objective**: Transform crude logging into surgical diagnostics

**Before/After Examples**:
```c
// Before: Basic, context-poor
DIAG_DEBUG("frame parsed");

// After: Surgical, context-rich
DIAG_DEBUG(DIAG_COMPONENT_FRAME_PARSER, STATUS_SUCCESS, 
          "Frame validation complete: length=%d, crc=0x%04X", length, crc);
```

**Implementation Strategy**:
- Systematically update each DIAG call in protocol files
- Add appropriate component identifiers
- Include relevant status codes
- Enhance message content with contextual data

### 3.4 Phase 4: Complete main.c Transformation
**Objective**: Demonstrate diagnostics framework capabilities

**Transformation Scope**:
- Remove all manual `debug_uart_*` calls
- Replace with appropriate `DIAG_*` calls
- Showcase system startup diagnostics
- Integrate nanopb test suite with diagnostics
- Demonstrate Oracle protocol readiness confirmation

---

## 4. Test-Driven Development (TDD) Validation Framework

### 4.1 Success Conditions Hierarchy

#### 4.1.1 Level 1: Basic Functionality
- [ ] **Compilation Success**: All nanpb→nanopb changes compile without errors
- [ ] **USART2 Output**: Diagnostics messages appear on PA2 at 115200 baud
- [ ] **USART1 Isolation**: Oracle protocol channel remains unaffected
- [ ] **Framework Initialization**: `bootloader_diag_init()` succeeds

#### 4.1.2 Level 2: Structured Logging
- [ ] **Timestamp Accuracy**: Microsecond-precision timestamps in output
- [ ] **Log Level Filtering**: ERROR, WARN, INFO, DEBUG, TRACE levels function correctly
- [ ] **Status Code Integration**: All status codes appear in appropriate contexts
- [ ] **Component Identification**: Module names correctly identify source systems

#### 4.1.3 Level 3: Oracle Protocol Integration  
- [ ] **Flow Step Tracking**: A-J protocol steps logged with timing
- [ ] **Error Diagnosis**: Protocol failures generate complete diagnostic context
- [ ] **Binary Inspection**: Frame data hex dumps available for analysis
- [ ] **Non-Interference**: Oracle protocol performance unaffected

#### 4.1.4 Level 4: Advanced Features
- [ ] **nanopb Integration**: Encode/decode operations fully logged
- [ ] **Memory Safety**: No buffer overflows or memory leaks
- [ ] **Performance Impact**: <5% overhead on protocol timing
- [ ] **Expandability**: Framework ready for additional output drivers

### 4.2 Validation Test Suite

#### 4.2.1 Hardware Validation Tests
```bash
# Basic USART2 functionality
./tools/run_test diagnostics_usart2_basic

# Oracle protocol isolation
./tools/run_test diagnostics_protocol_isolation  

# Timing precision validation
./tools/run_test diagnostics_timestamp_accuracy
```

#### 4.2.2 Protocol Integration Tests
```bash
# A-J flow tracking validation
./tools/run_test diagnostics_flow_tracking

# Error condition diagnostics
./tools/run_test diagnostics_error_handling

# Binary inspection capabilities
./tools/run_test diagnostics_hex_dump_validation
```

#### 4.2.3 System Integration Tests
```bash
# Complete Oracle protocol cycle with diagnostics
./tools/run_test bootloader_oracle_diagnostics_integrated

# nanopb operation diagnostics
./tools/run_test diagnostics_nanopb_integration

# Performance impact assessment
./tools/run_test diagnostics_performance_validation
```

### 4.3 Completion Criteria

#### 4.3.1 Functional Completion
- **All TDD tests pass** with 100% success rate
- **Oracle protocol debugging** transforms from guesswork to systematic analysis
- **Every protocol failure** generates complete diagnostic context
- **Zero interference** with Oracle protocol communication verified

#### 4.3.2 Quality Assurance  
- **Code review** confirms adherence to architectural principles
- **Documentation completeness** enables system recreation by other developers
- **Performance benchmarks** meet <5% overhead requirement
- **Memory usage analysis** confirms embedded system constraints met

#### 4.3.3 Future-Readiness
- **Modular architecture** validated for RTOS expansion
- **Output driver interface** tested for multiple destination capability
- **Configuration flexibility** demonstrated for development vs. production builds
- **Maintenance procedures** documented for long-term sustainability

---

## 5. Operational Excellence and Maintenance

### 5.1 Development Workflow Integration

#### 5.1.1 Daily Development Usage
- **Protocol debugging**: Developers use USART2 diagnostics for all Oracle protocol troubleshooting
- **Feature development**: New protocol features include appropriate DIAG calls from inception  
- **Bug reproduction**: Diagnostic output provides complete context for issue recreation
- **Performance optimization**: Timing information enables targeted performance improvements

#### 5.1.2 Production Deployment Considerations
- **Level filtering**: Production builds can disable DEBUG/TRACE levels for performance
- **Output driver selection**: Framework supports multiple output destinations
- **Security considerations**: Diagnostic content reviewed for sensitive information exposure
- **Resource management**: Memory usage monitored for embedded system constraints

### 5.2 Future Evolution Path

#### 5.2.1 RTOS Integration Readiness
- **Task context tracking**: Framework designed for multi-task diagnostic correlation
- **Shared memory diagnostics**: Architecture supports RTOS memory management debugging
- **Interrupt service routine logging**: ISR-safe logging mechanisms included
- **Priority-based filtering**: Diagnostic priorities aligned with RTOS task priorities

#### 5.2.2 Advanced Features Roadmap
- **Network diagnostics**: Framework extensible to TCP/IP or wireless protocols
- **File system logging**: Output drivers support SD card or flash file system storage  
- **Remote diagnostics**: Architecture supports diagnostic transmission to external systems
- **AI-assisted analysis**: Structured format enables machine learning diagnostic pattern analysis

### 5.3 Success Metrics and KPIs

#### 5.3.1 Developer Productivity Metrics
- **Debug time reduction**: Quantified improvement in Oracle protocol issue resolution time
- **Issue reproduction rate**: Percentage of bugs reproducible using diagnostic information  
- **Documentation quality**: Self-documenting nature of diagnostic output
- **Knowledge transfer efficiency**: New developers onboard faster with comprehensive diagnostics

#### 5.3.2 System Reliability Metrics
- **Mean Time To Resolution (MTTR)**: Faster issue resolution through precise diagnostics
- **False positive rate**: Minimal false alarms from diagnostic system
- **Coverage completeness**: Percentage of system operations with diagnostic coverage
- **Performance impact**: Verified minimal impact on real-time system performance

---

## Conclusion

The CockpitVM Modular Diagnostics Framework represents a **paradigm shift** from reactive debugging to proactive system health monitoring. By serving as the spiritual successor to flow_log while maintaining hardware-first reliability principles, this system transforms Oracle protocol development from art to science.

**Key Achievements**:
- **Surgical precision** in embedded system debugging
- **Absolute separation** between protocol and diagnostics channels  
- **Future-ready architecture** for RTOS and advanced feature expansion
- **Test-driven validation** ensuring reliability and maintainability

The framework's success will be measured not just in technical functionality, but in its ability to **accelerate development velocity**, **improve system reliability**, and **enable confident Oracle protocol evolution** within the CockpitVM ecosystem.

**Implementation Status**: Ready for systematic execution with clear success criteria and comprehensive validation framework.