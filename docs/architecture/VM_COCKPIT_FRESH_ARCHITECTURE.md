# CockpitVM Fresh Architecture Documentation

## Executive Summary

This document captures the complete architectural redesign of VM Cockpit, transitioning from a problematic Arduino HAL-based system to a clean, layered embedded native architecture. The redesign was driven by fundamental issues with clock configuration, timing conflicts, and abstraction layer violations that prevented reliable USART communication.

## Architectural Evolution

### Problem Statement
The original architecture suffered from:
- **Layering Violations**: Arduino HAL attempting system initialization
- **Timing Conflicts**: Multiple SysTick configurations fighting each other  
- **Clock Configuration Issues**: PLL timeout hangs due to improper initialization order
- **Pin Mapping Errors**: UART validation checking wrong pins (PA2/PA3 vs PA9/PA10)
- **Abstraction Confusion**: Mixed responsibilities between platform and compatibility layers

### Solution Approach
Complete architectural redesign following embedded systems best practices:
- **Clean Layer Separation**: Strict boundaries with single responsibilities
- **STM32 HAL First**: Platform layer as 90% adapter to proven STM32 HAL
- **Embedded Native API**: Professional naming conventions over Arduino compatibility
- **Fresh Implementation**: Zero legacy code to eliminate accumulated technical debt

## Layer Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Layer 6: Guest Application               │
│                      (Bytecode Programs)                    │
│  Authority: Application logic, VM instruction execution     │
│  Interface: ComponentVM instruction set (digitalWrite, etc) │
└─────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────┐
│                Layer 5: VM Hypervisor/Runtime               │
│                    (ComponentVM Core)                       │
│  Authority: Bytecode execution, resource management,        │
│             guest ↔ host translation                        │
│  Interface: Bytecode interpreter with instruction handlers  │
└─────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────┐
│              Layer 4: Host Interface (Embedded Native)      │
│                    (gpio_pin_write, uart_begin)             │
│  Authority: VM-to-hardware API translation                  │
│  Interface: Embedded native API (gpio_pin_write, etc)       │
└─────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────┐
│               Layer 3: Platform Layer (STM32G4)             │
│                  (stm32g4_gpio_write, etc)                  │
│  Authority: Hardware abstraction, STM32 HAL adapter         │
│  Interface: Platform-specific functions                     │
└─────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────┐
│             Layer 2: STM32 HAL (Vendor Library)             │
│               (HAL_GPIO_WritePin, HAL_UART_Transmit)        │
│  Authority: Chip-specific register abstractions             │
│  Interface: STM32 HAL API                                   │
└─────────────────────────────────────────────────────────────┘
                                  ↓
┌─────────────────────────────────────────────────────────────┐
│                Layer 1: Hardware (STM32G431CB)              │
│             (Physical GPIO, UART, Timers, Clocks)           │
│  Authority: Electrical signals and protocols                │
│  Interface: Memory-mapped registers                         │
└─────────────────────────────────────────────────────────────┘
```

## Layer Responsibilities & Authority

### Layer 6: Guest Application (Bytecode)
**Authority**: Application logic execution within VM sandbox
**Responsibilities**:
- User application logic written in C, compiled to ComponentVM bytecode
- Execution within VM protection boundaries
- Resource access only through hypervisor API

**Interface**: ComponentVM instruction set
```c
// Bytecode examples
PUSH 13, PUSH 1, CALL digitalWrite  // digitalWrite(13, HIGH)
PUSH 1000, CALL delay               // delay(1000)
```

### Layer 5: VM Hypervisor/Runtime (ComponentVM Core)
**Authority**: Bytecode execution, resource management, security enforcement
**Responsibilities**:
- Bytecode interpretation and instruction dispatch
- Memory protection and resource allocation
- Guest ↔ Host API translation
- Task scheduling (future: real-time scheduler + MPU)
- Resource arbitration (queues, mutexes for peripheral access)

**Interface**: Instruction handlers
```c
void handle_digitalWrite(uint8_t pin, bool state) {
    gpio_pin_write(pin, state);  // Calls Layer 5
}
```

### Layer 4: Host Interface (Embedded Native API)
**Authority**: Hardware abstraction for VM, embedded native API design
**Responsibilities**:
- Translate hypervisor requests to platform hardware operations
- Provide clean, professional embedded API
- Pin number to hardware mapping
- NO system initialization (assumes platform ready)

**Interface**: Embedded native functions
```c
// Professional embedded naming
void gpio_pin_write(uint8_t pin, bool state);
void uart_begin(uint32_t baud_rate);
void delay_ms(uint32_t milliseconds);
```

### Layer 3: Platform Layer (STM32G4 Adapter)
**Authority**: All hardware initialization, register access, system configuration
**Responsibilities**:
- Complete system initialization (clocks, HAL, peripherals)
- STM32 HAL adapter pattern (90% wrapper functions)
- Pin mapping tables (logical pin → GPIO port/pin)
- Hardware resource ownership

**Interface**: Platform-specific functions
```c
void stm32g4_platform_init(void);          // Complete system setup
void stm32g4_gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState state);
HAL_StatusTypeDef stm32g4_uart_transmit(uint8_t* data, uint16_t size);
```

### Layer 2: STM32 HAL (Vendor Library)
**Authority**: Chip-specific register abstractions, proven hardware drivers
**Responsibilities**:
- Register-level hardware abstraction
- Clock configuration (HAL_RCC_ClockConfig)
- Peripheral drivers (GPIO, UART, Timers)
- Interrupt handling

**Interface**: STM32 HAL API
```c
HAL_StatusTypeDef HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
```

### Layer 1: Hardware (STM32G474)
**Authority**: Physical hardware behavior, electrical characteristics
**Responsibilities**:
- Electrical signal generation and detection
- Clock generation and distribution
- Memory-mapped register interface
- Peripheral hardware implementation

## Key Architectural Principles

### 1. Strict Layer Boundaries
- Each layer only calls the layer directly below
- No layer skipping or direct hardware access from upper layers
- Clean dependency injection from bottom up

### 2. Single Responsibility per Layer
- **System Initialization**: Platform Layer only
- **API Translation**: Host Interface only  
- **Hardware Access**: Platform Layer via STM32 HAL only
- **Resource Management**: VM Hypervisor only

### 3. STM32 HAL First Strategy
- Platform layer is primarily adapter pattern to STM32 HAL
- Leverage proven vendor implementations
- Minimize custom register manipulation
- Defer to STM32 expertise for complex operations (clocks, timing)

### 4. Embedded Native API Design
- Professional embedded naming conventions
- Type-safe interfaces (uint8_t, bool vs Arduino's loose typing)
- Scalable patterns (gpio_pin_write, uart_begin, etc.)
- Self-documenting function names

## Implementation Structure

### File Organization
```
lib/vm_cockpit/src/
├── execution_engine/          # VM Core (Layer 6)
├── memory_manager/            # Memory protection (Layer 6)  
├── io_controller/             # I/O operations (Layer 6)
├── bridge_c/                  # C language bindings (Layer 6)
├── blackbox_observer/         # Internal telemetry hooks (Layer 6)
├── host_interface/            # Hardware abstraction (Layer 5)
│   ├── host_interface.h       # Embedded native API
│   ├── host_interface.c       # Implementation
│   ├── gpio.c                 # GPIO operations
│   ├── uart.c                 # UART operations  
│   └── timing.c               # Delay/timing
├── platform/stm32g4/          # STM32G4 platform adapter (Layer 4)
│   ├── stm32g4_platform.h     # Platform interface
│   ├── stm32g4_platform.c     # Core platform functions
│   ├── stm32g4_clock.c        # STM32 HAL clock config
│   ├── stm32g4_gpio.c         # STM32 HAL GPIO adapter
│   └── stm32g4_uart.c         # STM32 HAL UART adapter
└── semihosting/               # Debug support
```

### Data Flow Example: digitalWrite(13, HIGH)
```
1. Guest Bytecode: PUSH 13, PUSH 1, CALL digitalWrite
2. VM Hypervisor: handle_digitalWrite(13, true)
3. Host Interface: gpio_pin_write(13, true)
4. Platform Layer: stm32g4_gpio_write(GPIOC, GPIO_PIN_6, GPIO_PIN_SET)
5. STM32 HAL: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET)
6. Hardware: GPIOC->BSRR = (1 << 6)
```

## External Library Ecosystem

### vm_compiler (Separate Library)
**Purpose**: Host-side C-to-bytecode compilation tool
**Justification**: Different lifecycle (development vs runtime), different dependencies
**Interface**: Command-line tool, generates bytecode files

### vm_blackbox (Separate Library) 
**Purpose**: External production monitoring and telemetry analysis
**Justification**: Runs independently, loosely coupled to VM execution
**Interface**: Memory-mapped telemetry reading, external analysis tools

### Consolidated vm_cockpit (Unified Library)
**Purpose**: Complete VM runtime system
**Contents**: All execution-related components (Layers 5-6)
**Justification**: High cohesion, single versioning, atomic deployment

## Migration Strategy from Legacy

### Phase 1: Structural Migration
- Create new host_interface and platform directories
- Implement fresh embedded native API
- Build STM32 HAL-first platform layer

### Phase 2: Integration Testing  
- Replace arduino_hal calls with host_interface calls
- Test GPIO, UART, timing functions independently
- Validate layer boundaries

### Phase 3: Legacy Removal
- Remove arduino_hal directory completely
- Update io_controller to use host_interface
- Clean up include paths and dependencies

## Testing & Validation Strategy

### Layer Independence Testing
- Test each layer independently before integration
- Mock lower layers for unit testing
- Validate interface contracts

### Hardware-in-Loop Testing
- Direct register access tests (bypass all abstractions)
- Platform layer tests (STM32 HAL integration)
- End-to-end functionality tests

### Performance Validation
- Measure call stack overhead (4+ layer calls for GPIO)
- Profile timing-critical operations
- Ensure real-time constraints met

## Future Enhancements

### Capability-Based Security
- Resource handles for peripheral access
- Unforgeable capability tokens
- Fine-grained permission control

### Real-Time Scheduler Integration
- Task management in VM Hypervisor
- Preemptive scheduling with MPU protection
- Interrupt-safe resource management

### Multi-Platform Support
- Additional platform adapters (ESP32, etc.)
- Generic platform interface abstraction
- Cross-platform VM portability

## Lessons Learned

### Technical Insights
1. **Clock Configuration Complexity**: STM32 HAL clock config superior to custom register manipulation
2. **Layer Boundary Enforcement**: Strict boundaries prevent cascading failures
3. **Timing System Ownership**: Single authority for SysTick prevents conflicts
4. **Embedded Native APIs**: Professional naming scales better than Arduino compatibility

### Architectural Insights  
1. **Fresh Implementation Advantage**: Starting clean eliminates accumulated technical debt
2. **Adapter Pattern Power**: Platform layer as STM32 HAL adapter provides reliability + maintainability
3. **Separation of Concerns**: Each layer having single responsibility enables independent testing
4. **VM-First Design**: Building for hypervisor use case creates better abstractions than Arduino compatibility

This architecture provides a solid foundation for production embedded VM systems with clear scalability paths for advanced features like real-time scheduling, security, and multi-platform support.