# Golden Triangle Diagnostics Framework

**Phase 4.8**: DIAG Integration for Golden Triangle Tests

## Overview

This library provides structured diagnostics for Golden Triangle tests, extracted from the `vm_bootloader` diagnostics framework. It enables surgical debugging precision while maintaining complete separation from Oracle protocol communication.

## Key Features

- **Timestamped structured logging** via USART2 PA2/PA3 @ 115200 baud
- **Module/file/line tracking** with automatic context capture
- **Status code integration** for systematic error classification  
- **Zero interference** with Oracle protocol (USART1)
- **Mutual exclusion** with bootloader runtime console
- **Graceful degradation** - continues without output on UART failure

## Architecture

### Mutual Exclusion Enforcement
```c
#ifdef BOOTLOADER_RUNTIME_CONSOLE_ENABLED
    #error "Golden Triangle DIAG and Bootloader Runtime Console are mutually exclusive"
#endif
```

### Output Format
```
[timestamp] [level] [module] [file:line] [status] message
[00001250] [INFO ] [I2C_TEST] [test_i2c.c:45] [SUCCESS] Device detected at 0x3C
```

## Usage

### Initialization (Automatic via Golden Triangle)
```c
#ifdef GOLDEN_TRIANGLE_DIAG_ENABLED
    gt_diag_init(NULL, 115200);  // Uses default USART2 driver
#endif
```

### Logging Macros
```c
GT_DIAG_INFO(GT_MOD_I2C_TEST, "Starting I2C scan");
GT_DIAG_ERROR_STATUS(GT_MOD_I2C_TEST, STATUS_ERROR_I2C, "Device not found at 0x%02X", addr);
GT_DIAG_DEBUG(GT_MOD_I2C_TEST, "Scan complete: %d devices found", count);
```

### Flow Tracking
```c
GT_DIAG_FLOW('A', "I2C initialization");
GT_DIAG_FLOW('B', "Device detection");
GT_DIAG_FLOW('C', "Data transmission");
```

### Binary Data Inspection
```c
uint8_t i2c_data[] = {0x00, 0xAE, 0x00, 0xAF};
GT_DIAG_BUFFER("I2C Commands", i2c_data, sizeof(i2c_data));
```

## Integration

### Test Catalog Configuration
```yaml
test_example:
  diag_enabled: true  # Enables GOLDEN_TRIANGLE_DIAG_ENABLED
  semihosting: false  # Use DIAG instead
```

### Dependencies
- **vm_cockpit platform layer**: `stm32g4_debug_uart_*`, `stm32g4_get_tick_ms`
- **STM32G4 HAL**: Already available in Golden Triangle tests
- **No coupling**: Independent of vm_bootloader protocol/protobuf/CRC components

## Module Identifiers

| Module | Description |
|--------|-------------|
| `GT_MOD_I2C_TEST` | I2C peripheral tests |
| `GT_MOD_UART_TEST` | UART peripheral tests |
| `GT_MOD_GPIO_TEST` | GPIO peripheral tests |
| `GT_MOD_SPI_TEST` | SPI peripheral tests |
| `GT_MOD_GENERAL` | General test operations |

## Status Codes

| Code | Description |
|------|-------------|
| `STATUS_SUCCESS` | Operation completed successfully |
| `STATUS_ERROR_I2C` | I2C communication failure |
| `STATUS_ERROR_UART` | UART communication failure |
| `STATUS_ERROR_GPIO` | GPIO configuration failure |
| `STATUS_ERROR_TIMEOUT` | Operation timeout |
| `STATUS_ERROR_HARDWARE` | Hardware fault detected |

## Origin

Extracted from `lib/vm_bootloader/include/bootloader_diagnostics.h` and `lib/vm_bootloader/src/bootloader_diagnostics.c` with Golden Triangle-specific adaptations and mutual exclusion safeguards.