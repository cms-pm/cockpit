# Trinity: Zero-Cost Hardware Abstraction Architecture
*Three-Tier Template System for CockpitVM Phase 4.9*

## Executive Summary

This document defines **Trinity** - a research implementation of three-tier zero-cost hardware abstraction for CockpitVM, exploring techniques to achieve **bare-metal performance with hardware independence**. The Trinity architecture aims to eliminate function call overhead for common operations while maintaining cross-platform portability through compile-time template metaprogramming.

**Research Goal**: `digitalWrite(13, HIGH)` compiles to **single instruction**: `GPIOC->BSRR = GPIO_PIN_6`

## Architectural Foundation

### Design Principles

1. **Zero-Cost Abstraction**: Templates eliminate runtime overhead completely
2. **Compile-Time Hardware Discovery**: Static analysis of VM bytecode determines used peripherals  
3. **Platform-Specific Optimization**: Architecture-aware register access patterns
4. **CppCon-Inspired Elegance**: Pure template metaprogramming without artificial memory sections
5. **Counter-Guidance Welcome**: Technical correctness over initial assumptions

### Inspiration Sources

- **CppCon 2024 Performance Example**: Template-based HAL with C++20 concepts
- **Zephyr RTOS Device Tree**: Hardware description separation
- **Embedded Linux Register Mapping**: Memory-mapped I/O abstraction
- **STM32 HAL Integration**: Proven vendor code preservation

## Trinity: The Three-Tier Architecture

```
┌─────────────────────────────────────────────────┐
│ Tier 1: Template Hardware Descriptors          │
│ Target: 90% of operations - Zero runtime cost  │
│ Pattern: constexpr register access templates   │
└─────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│ Tier 2: Runtime HAL Integration               │
│ Target: 9% of operations - Performance fallback│
│ Pattern: Concept-based vendor HAL dispatch     │
└─────────────────────────────────────────────────┘
                          ↓
┌─────────────────────────────────────────────────┐
│ Tier 3: Generic Register Interface             │
│ Target: 1% edge cases - Compatibility only     │
│ Pattern: Runtime register manipulation         │
└─────────────────────────────────────────────────┘
```

### Tier 1: Template Hardware Descriptors

**Purpose**: Achieve zero function call overhead for common GPIO, UART, I2C operations

```cpp
// Platform-specific template definitions
template<uintptr_t gpio_base, uint32_t pin_mask>
class STM32Pin {
    // Runtime-optimized approach - direct register access
    static constexpr void write(bool state) noexcept {
        *reinterpret_cast<volatile uint32_t*>(gpio_base + 0x18) = 
            state ? pin_mask : (pin_mask << 16);  // BSRR atomic set/reset
    }
    
    static constexpr bool read() noexcept {
        return (*reinterpret_cast<volatile uint32_t*>(gpio_base + 0x10) & pin_mask) != 0; // IDR
    }
};

// Platform-specific pin aliases (STM32G4 WeAct board)
using PA5 = STM32Pin<GPIOA_BASE, GPIO_PIN_5>;   // Debug pin
using PC6 = STM32Pin<GPIOC_BASE, GPIO_PIN_6>;   // Pin 13 LED
```

**Compilation Result**:
```asm
; digitalWrite(13, HIGH) becomes:
mov r1, #64          ; GPIO_PIN_6 = 0x40
str r1, [r0, #24]    ; GPIOC->BSRR = r1
```

### Tier 2: Runtime HAL Integration

**Purpose**: Concept-based fallback for complex operations maintaining vendor HAL

```cpp
template<typename HalImpl>
class GenericPeripheral {
    template<uint32_t config>
    static constexpr bool configure() {
        return HalImpl::template configure_impl<config>();
    }
};

// STM32 HAL integration
struct STM32_UART_Impl {
    template<uint32_t baud_rate>
    static bool configure_impl() {
        // Use proven STM32 HAL for complex initialization
        return HAL_UART_Init(&huart_config) == HAL_OK;
    }
};
```

### Tier 3: Generic Register Interface

**Purpose**: Compatibility layer for unknown hardware configurations

```cpp
// Fallback for runtime-determined operations
namespace GenericHAL {
    bool gpio_write_fallback(uint8_t pin, bool state) {
        // Runtime pin lookup and register manipulation
        return platform_gpio_write(pin, state);
    }
}
```

## Architecture-Specific Optimizations

### ARM Cortex-M (STM32G4) Advantages

**Register Access Patterns**:
- **BSRR Atomic Operations**: Set/reset in single instruction
- **Immediate Loading**: ARM excels at constant loading
- **ART Cache Optimization**: Sequential template placement

```cpp
// ARM-optimized GPIO template
template<GPIO_TypeDef* Port, uint32_t Pin>
class ARMGpioPin {
    static constexpr void write(bool state) {
        // Single 32-bit store - ARM specialty
        Port->BSRR = state ? Pin : (Pin << 16);
    }
};
```

### Xtensa (ESP32) Optimization

**Register Architecture Adaptation**:
- **Different GPIO Layout**: Separate registers for set/clear
- **Bit Manipulation Instructions**: Xtensa-specific optimizations

```cpp
// Xtensa-optimized GPIO template
template<uint32_t gpio_num>
class XtensaGpioPin {
    static constexpr void write(bool state) {
        if constexpr (gpio_num < 32) {
            if (state) {
                GPIO.out_w1ts = BIT(gpio_num);  // Write-1-to-set
            } else {
                GPIO.out_w1tc = BIT(gpio_num);  // Write-1-to-clear
            }
        }
        // Handle GPIO32+ with different registers
    }
};
```

### RISC-V Characteristics

**Instruction Set Considerations**:
- **Load-Immediate Limitations**: Multi-instruction constants
- **Register Efficiency**: Template constant optimization

```cpp
// RISC-V-optimized templates
template<uintptr_t base, uint32_t offset, uint32_t mask>
class RISCVPeripheral {
    // Compiler optimizes to optimal RISC-V instruction sequence
    static constexpr void write_bit(bool state) {
        volatile uint32_t* reg = reinterpret_cast<volatile uint32_t*>(base + offset);
        if (state) {
            *reg |= mask;
        } else {
            *reg &= ~mask;
        }
    }
};
```

## Hardware Discovery Mechanism

### Static Bytecode Analysis (Selected Approach)

**Rationale**: CppCon uses explicit instantiation, but CockpitVM requires automated discovery from compiled guest programs.

```python
# scripts/cockpit_template_generator.py
def analyze_pin_usage(bytecode_path):
    """Analyze CVBC file to extract hardware requirements"""
    with open(bytecode_path, 'rb') as f:
        cvbc_data = parse_cvbc_format(f.read())
    
    # Extract metadata-declared pin usage
    used_pins = []
    for pin_config in cvbc_data.metadata.pin_configs:
        if pin_config.pin_function == PIN_FUNCTION_GPIO:
            used_pins.append({
                'port': pin_config.port,
                'pin': pin_config.pin_number,
                'function': pin_config.pin_function
            })
    
    # Generate platform-specific templates
    generate_pin_templates(used_pins, cvbc_data.metadata.target_variant)
    return used_pins
```

**Template Generation Output**:
```cpp
// Generated: lib/vm_cockpit/src/hardware/generated_pins.hpp
// Auto-generated from guest program analysis - DO NOT EDIT

#pragma once
#include "hardware/stm32g4/stm32g4_pin_templates.hpp"

namespace CockpitHAL {
    // Only pins used by guest programs
    using PA5 = STM32Pin<GPIOA_BASE, GPIO_PIN_5>;  // Found in SOS program
    using PC6 = STM32Pin<GPIOC_BASE, GPIO_PIN_6>;  // Found in SOS program
    // Unused pins not instantiated - zero code bloat
}
```

## VM Integration Architecture

### Complete Handler Replacement (Selected Approach)

Replace existing `ExecutionEngine::handle_digital_write()` completely with template-optimized version:

```cpp
VM::HandlerResult ExecutionEngine::handle_digital_write(uint8_t flags, uint16_t immediate, 
                                                       MemoryManager& memory, IOController& io) noexcept {
    int32_t pin, state;
    if (!pop(state) || !pop(pin)) {
        return VM::HandlerResult(VM_ERROR_STACK_UNDERFLOW);
    }
    
    // Compile-time template dispatch - zero runtime overhead
    switch (pin) {
        case 5:  if constexpr (requires { CockpitHAL::PA5{}; }) {
                    CockpitHAL::PA5{}.write(state != 0);
                    return VM::HandlerResult(VM::HandlerReturn::CONTINUE_NO_CHECK);
                 }
                 break;
        case 13: if constexpr (requires { CockpitHAL::PC6{}; }) {
                    CockpitHAL::PC6{}.write(state != 0);
                    return VM::HandlerResult(VM::HandlerReturn::CONTINUE_NO_CHECK);
                 }
                 break;
    }
    
    // Compile-time error for unmapped pins
    static_assert(false, "GPIO pin not configured in guest program metadata");
}
```

### Error Handling Strategy

**Compile-Time Validation**: Errors caught during compilation, not runtime

```cpp
// Template constraints ensure type safety
template<typename PinType>
requires GPIOPinConcept<PinType>
VM::HandlerResult handle_gpio_operation(PinType& pin, bool state) {
    pin.write(state);
    return VM::HandlerResult(VM::HandlerReturn::CONTINUE_NO_CHECK);
}
```

## Clock Management Integration

### Platform Layer Responsibility

GPIO clock enablement managed in platform initialization, always available before pin operations:

```cpp
// lib/vm_cockpit/src/platform/stm32g4/stm32g4_platform.c
void stm32g4_platform_init(void) {
    // Always enable all GPIO clocks - minimal power impact
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // Platform-specific initialization
    SystemClock_Config();
    HAL_Init();
}
```

## Peripheral Priority Implementation

### Phase 4.9 Scope

1. **GPIO** (Phase 4.9.1): Foundation template system
2. **UART** (Phase 4.9.2): Template-optimized serial communication  
3. **I2C** (Phase 4.9.3): Multi-device bus management
4. **UART_RX Interrupts** (Phase 4.9.4): Interrupt-driven receive
5. **SPI** (Phase 5.0): After I2C validation

### UART Template Example

```cpp
template<USART_TypeDef* Instance, uint32_t BaudRate>
class STM32_UART {
    static constexpr void transmit_byte(uint8_t data) {
        // Wait for TXE flag
        while (!(Instance->ISR & USART_ISR_TXE));
        Instance->TDR = data;  // Single register write
    }
    
    static constexpr bool data_available() {
        return (Instance->ISR & USART_ISR_RXNE) != 0;
    }
};

using DebugUART = STM32_UART<USART2, 115200>;  // Compile-time configuration
```

## Build Integration Architecture

### PlatformIO Custom Build Stage (Selected Approach)

**Implementation**: Option C provides optimal timing and data access

```python
# scripts/cockpit_template_generator.py
Import("env")

def analyze_and_generate(source, target, env):
    """
    PlatformIO build hook - runs after guest compilation, before VM compilation
    """
    
    # 1. Locate compiled guest programs
    guest_programs = find_cvbc_files("guest_programs/")
    
    # 2. Analyze each program's hardware requirements
    all_used_pins = set()
    all_used_peripherals = set()
    
    for program_path in guest_programs:
        metadata = parse_cvbc_metadata(program_path)
        all_used_pins.update(extract_pin_usage(metadata))
        all_used_peripherals.update(extract_peripheral_usage(metadata))
    
    # 3. Generate platform-specific templates
    target_platform = env.BoardConfig().get("build.mcu", "STM32G431")
    generate_pin_templates(all_used_pins, target_platform)
    generate_peripheral_templates(all_used_peripherals, target_platform)
    
    # 4. Generate optimized VM dispatch tables
    generate_vm_handlers(all_used_pins, all_used_peripherals)
    
    print(f"Generated templates for {len(all_used_pins)} pins, {len(all_used_peripherals)} peripherals")

# Hook into build process
env.AddPreAction("$BUILD_DIR/lib/vm_cockpit/libvm_cockpit.a", analyze_and_generate)
```

**Build Flow**:
1. Guest programs compiled to CVBC format
2. Python script analyzes CVBC metadata  
3. Templates generated for discovered hardware
4. VM compiled with optimized templates
5. Final binary contains only used hardware abstractions

## Future Expansion Considerations

### Multi-Platform Template Hierarchies

```cpp
// Cross-platform abstraction layer
namespace UniversalHAL {
    template<typename PlatformPin>
    concept GPIOPinConcept = requires(PlatformPin pin, bool state) {
        { pin.write(state) } -> std::same_as<void>;
        { pin.read() } -> std::same_as<bool>;
    };
    
    // Platform-agnostic VM integration
    template<GPIOPinConcept PinType>
    VM::HandlerResult handle_gpio_write(PinType& pin, bool state) {
        pin.write(state);
        return VM::HandlerResult(VM::HandlerReturn::CONTINUE_NO_CHECK);
    }
}
```

### Security Integration

Templates maintain compile-time security validation:

```cpp
template<typename PinType>
requires (PinType::security_level >= SECURITY_LEVEL_PRODUCTION)
class SecureGPIO {
    static constexpr void write(bool state) {
        // Compile-time security policy enforcement
        PinType::write(state);
    }
};
```

## Success Metrics

- **Performance**: GPIO operations compile to 1-2 instructions
- **Memory**: Only used peripherals instantiated (zero bloat)
- **Portability**: Same VM bytecode runs on ARM/Xtensa/RISC-V
- **Maintainability**: Platform-specific code isolated in templates
- **Security**: Compile-time validation prevents runtime errors

## Conclusion

The Trinity architecture explores the challenging goal of embedded systems: hardware independence with bare-metal performance. The three-tier template-based approach eliminates abstraction overhead while the CVBC metadata format enables automated optimization discovery.

Trinity explores scaling from simple GPIO operations to complex multi-peripheral coordination, investigating foundations for advanced embedded hypervisor architectures with zero-cost hardware abstraction techniques.