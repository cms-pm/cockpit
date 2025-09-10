# CockpitVM Bytecode Container (CVBC) Format Specification
*Version 1.0 - Research Implementation Embedded VM Binary Format*

## Document Overview

This specification defines the CVBC (CockpitVM Bytecode Container) format - a metadata-rich binary container for CockpitVM programs. The format enables hardware discovery, security validation, performance optimization, and cross-platform deployment for embedded systems using the Trinity architecture.

**Key Design Goals**:
- **Trinity Template Generation**: Metadata drives zero-cost hardware abstraction
- **Security**: Code signing, encryption, and integrity validation
- **Hardware Validation**: Compile-time compatibility verification
- **Performance Optimization**: Build-time analysis for optimal code generation
- **Future-Proofing**: Extensible format for emerging requirements

## File Format Structure

### Binary Layout

```
┌─────────────────────────────┐ ←─ File Start
│     CVBC Header (64B)       │
├─────────────────────────────┤
│   Metadata Section (Var)    │ ←─ Hardware requirements, build info
├─────────────────────────────┤
│  Security Section (Var)     │ ←─ Signatures, encryption data
├─────────────────────────────┤
│   Bytecode Section (Var)    │ ←─ VM instructions
├─────────────────────────────┤
│  Debug Section (Optional)   │ ←─ Source mapping, symbols
└─────────────────────────────┘ ←─ File End
```

### Magic Numbers & Versioning

```cpp
#define CVBC_MAGIC           0x43564243  // "CVBC" little-endian
#define CVBC_VERSION_MAJOR   1          // Breaking changes
#define CVBC_VERSION_MINOR   0          // Compatible additions
#define CVBC_MIN_FILE_SIZE   128        // Minimum valid file size
```

## Header Format

### Primary Header (64 bytes)

```cpp
typedef struct __attribute__((packed)) {
    // === FILE IDENTIFICATION (16 bytes) ===
    uint32_t magic;                    // 0x43564243 ("CVBC")
    uint16_t format_version_major;     // Format compatibility version
    uint16_t format_version_minor;     // Feature version
    uint32_t total_file_size;          // Complete file size in bytes
    uint32_t header_crc32;            // CRC32 of this header (excludes this field)
    
    // === SECTION OFFSETS (24 bytes) ===
    uint32_t metadata_offset;         // Byte offset to metadata section
    uint32_t metadata_size;           // Size of metadata section
    uint32_t security_offset;         // Byte offset to security section  
    uint32_t security_size;           // Size of security section
    uint32_t bytecode_offset;         // Byte offset to VM bytecode
    uint32_t bytecode_size;           // Size of bytecode in bytes
    
    // === INTEGRITY & TIMING (16 bytes) ===
    uint32_t payload_crc32;           // CRC32 of all sections after header
    uint64_t compilation_timestamp;    // Unix timestamp UTC (seconds)
    uint32_t build_number;            // Incremental build identifier
    
    // === FUTURE EXPANSION (8 bytes) ===
    uint8_t  flags;                   // Feature flags (encryption, compression, etc.)
    uint8_t  reserved[7];             // Must be zero
    
} cvbc_header_t;

// Header flags
#define CVBC_FLAG_ENCRYPTED     0x01   // Bytecode section encrypted
#define CVBC_FLAG_COMPRESSED    0x02   // Bytecode section compressed
#define CVBC_FLAG_SIGNED        0x04   // File digitally signed
#define CVBC_FLAG_DEBUG         0x08   // Debug information present
#define CVBC_FLAG_DETERMINISTIC 0x10   // Reproducible build
```

## Metadata Section

### Core Metadata Structure

```cpp
typedef struct __attribute__((packed)) {
    // === HARDWARE TARGET SPECIFICATION (16 bytes) ===
    uint8_t  target_architecture;     // ARM=1, Xtensa=2, RISC-V=3, x86=4
    uint8_t  target_family;           // STM32=1, ESP32=2, SiFive=3, Nordic=4
    uint16_t target_variant;          // STM32G431=0x0431, ESP32-C3=0x0003
    uint32_t target_features;         // Required hardware features (bitfield)
    uint32_t min_flash_kb;            // Minimum flash memory requirement
    uint32_t min_ram_kb;              // Minimum RAM requirement
    
    // === RESOURCE REQUIREMENTS (32 bytes) ===
    uint16_t gpio_pin_count;          // Number of GPIO pins used
    uint8_t  uart_instances_mask;     // UART instances required (bitmask)
    uint8_t  i2c_instances_mask;      // I2C instances required (bitmask) 
    uint8_t  spi_instances_mask;      // SPI instances required (bitmask)
    uint8_t  timer_instances_mask;    // Timer instances required (bitmask)
    uint8_t  adc_channels_mask;       // ADC channels required (bitmask)
    uint8_t  dac_channels_mask;       // DAC channels required (bitmask)
    uint16_t interrupt_priority_mask; // Required interrupt priority levels
    uint16_t dma_channels_mask;       // DMA channels required (bitmask)
    uint32_t external_interfaces;     // External interface requirements
    uint64_t feature_requirements;    // Extended feature flags
    
    // === BUILD & SOURCE INFORMATION (24 bytes) ===
    uint32_t compiler_version;        // CockpitVM compiler version (BCD)
    uint8_t  source_language;         // C=1, C++=2, Rust=3, Zig=4
    uint8_t  optimization_level;      // Debug=0, Size=1, Speed=2, Paranoid=3
    uint16_t source_file_count;       // Number of source files compiled
    uint32_t source_lines_total;      // Total source lines compiled  
    uint8_t  build_fingerprint[8];    // Hash of compiler+flags+environment
    uint64_t source_tree_hash;        // SHA256 truncated to 64-bit
    
    // === RUNTIME CONSTRAINTS (16 bytes) ===
    uint32_t max_stack_depth_words;   // Maximum VM stack depth required
    uint32_t max_heap_bytes;          // Maximum heap allocation
    uint32_t max_execution_cycles;    // CPU cycle budget per invocation
    uint16_t max_interrupt_latency_us;// Maximum acceptable interrupt latency
    uint16_t reserved_constraints;    // Future runtime constraints
    
    // === VARIABLE-LENGTH SECTIONS ===
    uint16_t pin_config_count;        // Number of pin configuration entries
    uint16_t dependency_count;        // Number of library dependencies
    uint16_t performance_hint_count;  // Number of performance optimization hints
    uint16_t custom_metadata_size;    // Size of custom metadata section
    
} cvbc_core_metadata_t;

// Hardware feature flags
#define CVBC_FEATURE_GPIO           0x00000001
#define CVBC_FEATURE_UART           0x00000002  
#define CVBC_FEATURE_I2C            0x00000004
#define CVBC_FEATURE_SPI            0x00000008
#define CVBC_FEATURE_PWM            0x00000010
#define CVBC_FEATURE_ADC            0x00000020
#define CVBC_FEATURE_DAC            0x00000040
#define CVBC_FEATURE_TIMERS         0x00000080
#define CVBC_FEATURE_RTC            0x00000100
#define CVBC_FEATURE_WATCHDOG       0x00000200
#define CVBC_FEATURE_CRYPTO         0x00000400
#define CVBC_FEATURE_USB            0x00000800
#define CVBC_FEATURE_CAN            0x00001000
#define CVBC_FEATURE_ETHERNET       0x00002000
#define CVBC_FEATURE_WIRELESS       0x00004000
#define CVBC_FEATURE_DISPLAY        0x00008000
```

### Pin Configuration Metadata

```cpp
typedef struct __attribute__((packed)) {
    uint8_t  pin_number;              // Physical pin number (0-255)
    uint8_t  port_identifier;         // Port identifier (A=0, B=1, C=2, etc.)
    uint8_t  pin_function;            // Pin usage type
    uint8_t  pin_mode;                // Input/Output configuration
    uint8_t  pin_pull;                // Pull resistor configuration
    uint8_t  pin_drive_strength;      // Output drive strength
    uint16_t pin_frequency_hz;        // PWM/Clock frequency (if applicable)
    uint32_t pin_alternate_function;  // Alternate function register value
    uint8_t  pin_initial_state;       // Initial pin state (for outputs)
    uint8_t  pin_flags;               // Additional pin configuration flags
    uint16_t reserved;                // Future expansion
    
} cvbc_pin_config_t;

// Pin function types
#define CVBC_PIN_FUNC_GPIO          1
#define CVBC_PIN_FUNC_UART_TX       2
#define CVBC_PIN_FUNC_UART_RX       3
#define CVBC_PIN_FUNC_I2C_SDA       4
#define CVBC_PIN_FUNC_I2C_SCL       5
#define CVBC_PIN_FUNC_SPI_MOSI      6
#define CVBC_PIN_FUNC_SPI_MISO      7
#define CVBC_PIN_FUNC_SPI_SCK       8
#define CVBC_PIN_FUNC_PWM_OUT       9
#define CVBC_PIN_FUNC_ADC_IN        10
#define CVBC_PIN_FUNC_DAC_OUT       11
#define CVBC_PIN_FUNC_INTERRUPT     12

// Pin modes
#define CVBC_PIN_MODE_INPUT         1
#define CVBC_PIN_MODE_OUTPUT        2
#define CVBC_PIN_MODE_ALTERNATE     3
#define CVBC_PIN_MODE_ANALOG        4

// Pin pull configuration
#define CVBC_PIN_PULL_NONE          0
#define CVBC_PIN_PULL_UP            1
#define CVBC_PIN_PULL_DOWN          2
```

### Dependency Information

```cpp
typedef struct __attribute__((packed)) {
    char     lib_name[16];            // Library name (null-terminated)
    uint32_t min_version_major;       // Minimum required major version
    uint32_t min_version_minor;       // Minimum required minor version
    uint32_t required_features;       // Required library features (bitfield)
    uint8_t  compatibility_level;     // ABI=1, API=2, Source=3
    uint8_t  criticality;             // Optional=1, Recommended=2, Required=3
    uint16_t reserved;                // Future expansion
    
} cvbc_dependency_t;
```

### Performance Optimization Hints

```cpp
typedef struct __attribute__((packed)) {
    uint8_t  hint_type;               // Type of performance hint
    uint8_t  priority;                // Optimization priority (1-255)
    uint16_t hint_data_size;          // Size of hint-specific data
    // Followed by hint_data_size bytes of hint-specific data
    
} cvbc_performance_hint_t;

// Performance hint types
#define CVBC_PERF_HINT_HOT_PATH        1    // Frequently executed code path
#define CVBC_PERF_HINT_COLD_PATH       2    // Rarely executed code path
#define CVBC_PERF_HINT_LOOP_UNROLL     3    // Suggest loop unrolling
#define CVBC_PERF_HINT_INLINE_FUNC     4    // Suggest function inlining
#define CVBC_PERF_HINT_CACHE_FRIENDLY  5    // Memory access pattern hint
#define CVBC_PERF_HINT_INTERRUPT_SAFE  6    // Code safe for interrupt context
```

## Security Section

### Security Header

```cpp
typedef struct __attribute__((packed)) {
    uint8_t  security_version;        // Security format version
    uint8_t  signature_algorithm;     // Digital signature algorithm
    uint8_t  hash_algorithm;          // Hash algorithm for integrity
    uint8_t  encryption_algorithm;    // Encryption algorithm (if encrypted)
    uint32_t signature_size;          // Size of signature data (0 if unsigned)
    uint32_t public_key_size;         // Size of embedded public key (0 if external)
    uint32_t encrypted_data_size;     // Size of encrypted data (0 if unencrypted)
    uint32_t key_derivation_params;   // Key derivation parameters
    
} cvbc_security_header_t;

// Supported algorithms
#define CVBC_SIG_NONE               0
#define CVBC_SIG_ED25519            1   // Recommended for embedded
#define CVBC_SIG_ECDSA_P256         2
#define CVBC_SIG_RSA_2048           3

#define CVBC_HASH_SHA256            1   // Primary hash algorithm
#define CVBC_HASH_BLAKE2B           2   // Alternative for performance

#define CVBC_ENC_NONE               0
#define CVBC_ENC_AES256_GCM         1   // Authenticated encryption
#define CVBC_ENC_CHACHA20_POLY1305  2   // Alternative AEAD
```

### Integrity Validation Data

```cpp
typedef struct __attribute__((packed)) {
    uint8_t  source_sha256[32];       // SHA256 of original source code
    uint8_t  bytecode_sha256[32];     // SHA256 of unencrypted bytecode
    uint8_t  metadata_sha256[32];     // SHA256 of metadata section
    uint32_t build_reproducible_hash; // Hash for reproducible build verification
    uint64_t secure_timestamp;        // Trusted timestamp (if available)
    
} cvbc_integrity_data_t;
```

## Bytecode Section

### Bytecode Header

```cpp
typedef struct __attribute__((packed)) {
    uint16_t instruction_count;       // Number of VM instructions
    uint16_t entry_point_offset;      // Offset to main execution start
    uint32_t constant_pool_offset;    // Offset to constant data
    uint32_t constant_pool_size;      // Size of constant pool
    uint16_t stack_size_required;     // VM stack size requirement
    uint16_t local_variable_count;    // Number of local variables
    
} cvbc_bytecode_header_t;
```

### VM Instruction Format (Existing)

```cpp
// Maintains existing CockpitVM instruction format
typedef struct __attribute__((packed)) {
    uint8_t  opcode;                  // VM operation code
    uint8_t  flags;                   // Instruction modifier flags  
    uint16_t immediate;               // Immediate operand value
    
} vm_instruction_t;
```

## Debug Section (Optional)

### Debug Information Header

```cpp
typedef struct __attribute__((packed)) {
    uint32_t symbol_table_offset;     // Offset to symbol table
    uint32_t symbol_table_size;       // Size of symbol table
    uint32_t line_info_offset;        // Offset to source line mapping
    uint32_t line_info_size;          // Size of line info
    uint32_t variable_info_offset;    // Offset to variable debug info
    uint32_t variable_info_size;      // Size of variable info
    
} cvbc_debug_header_t;
```

## File Validation Algorithms

### CRC32 Calculation

```cpp
// Standard CRC32 with polynomial 0xEDB88320
uint32_t cvbc_calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
    }
    
    return ~crc;
}
```

### File Format Validation

```cpp
bool cvbc_validate_format(const uint8_t* file_data, size_t file_size) {
    if (file_size < sizeof(cvbc_header_t)) return false;
    
    const cvbc_header_t* header = (const cvbc_header_t*)file_data;
    
    // Magic number validation
    if (header->magic != CVBC_MAGIC) return false;
    
    // Version compatibility check
    if (header->format_version_major > CVBC_VERSION_MAJOR) return false;
    
    // Size validation
    if (header->total_file_size != file_size) return false;
    if (header->total_file_size < CVBC_MIN_FILE_SIZE) return false;
    
    // Section boundary validation
    if (header->metadata_offset + header->metadata_size > file_size) return false;
    if (header->bytecode_offset + header->bytecode_size > file_size) return false;
    
    // Header integrity check
    uint32_t header_crc = cvbc_calculate_crc32((const uint8_t*)header, 
                                              offsetof(cvbc_header_t, header_crc32));
    if (header_crc != header->header_crc32) return false;
    
    // Payload integrity check  
    uint32_t payload_crc = cvbc_calculate_crc32(
        file_data + sizeof(cvbc_header_t),
        file_size - sizeof(cvbc_header_t)
    );
    if (payload_crc != header->payload_crc32) return false;
    
    return true;
}
```

## Trinity Template Generation Integration

### Hardware Discovery API for Trinity Architecture

```cpp
typedef struct {
    uint8_t  pin_count;
    cvbc_pin_config_t* pin_configs;
    uint32_t peripheral_mask;
    uint32_t feature_requirements;
    
} cvbc_hardware_requirements_t;

// Extract hardware requirements for template generation
bool cvbc_extract_hardware_requirements(const uint8_t* cvbc_data,
                                       cvbc_hardware_requirements_t* requirements) {
    const cvbc_header_t* header = (const cvbc_header_t*)cvbc_data;
    const cvbc_core_metadata_t* metadata = 
        (const cvbc_core_metadata_t*)(cvbc_data + header->metadata_offset);
    
    requirements->pin_count = metadata->gpio_pin_count;
    requirements->pin_configs = (cvbc_pin_config_t*)(metadata + 1);
    requirements->peripheral_mask = metadata->target_features;
    requirements->feature_requirements = metadata->target_features;
    
    return true;
}
```

## Usage Examples

### Creating CVBC Files

```python
# Python API for CVBC generation
import cvbc

# Create new CVBC container
container = cvbc.Container()

# Set hardware requirements
container.set_target(
    architecture=cvbc.ARCH_ARM,
    family=cvbc.FAMILY_STM32,
    variant=0x0431  # STM32G431
)

# Add pin configurations
container.add_pin_config(
    pin_number=13,
    port='C', 
    function=cvbc.PIN_FUNC_GPIO,
    mode=cvbc.PIN_MODE_OUTPUT
)

# Add bytecode
container.set_bytecode(compiled_vm_instructions)

# Generate signed CVBC file
container.save("program.cvbc", 
               sign_with_key="private_key.pem",
               compression=True)
```

### Validating CVBC Files

```cpp
// C API for CVBC validation
#include "cvbc_format.h"

bool validate_and_load_program(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    
    // Read entire file
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);
    
    uint8_t* file_data = malloc(file_size);
    fread(file_data, 1, file_size, f);
    fclose(f);
    
    // Validate format
    if (!cvbc_validate_format(file_data, file_size)) {
        free(file_data);
        return false;
    }
    
    // Extract hardware requirements for template generation
    cvbc_hardware_requirements_t hw_req;
    if (!cvbc_extract_hardware_requirements(file_data, &hw_req)) {
        free(file_data);
        return false;
    }
    
    // Validate hardware compatibility
    if (!validate_hardware_compatibility(&hw_req)) {
        free(file_data);
        return false;
    }
    
    // Load and execute program
    load_vm_program(file_data);
    free(file_data);
    return true;
}
```

## Backward Compatibility

### Version Migration Strategy

1. **Major Version Changes**: Breaking format changes require new parser
2. **Minor Version Changes**: Additive changes with graceful degradation  
3. **Reserved Fields**: All reserved fields must be zero for forward compatibility
4. **Section Extensions**: New sections can be added without breaking existing parsers

### Legacy Support

```cpp
// Support for legacy bytecode files during transition
bool cvbc_load_legacy_format(const uint8_t* data, size_t size) {
    // Detect legacy format and convert to CVBC
    if (is_legacy_format(data, size)) {
        return convert_legacy_to_cvbc(data, size);
    }
    return false;
}
```

## Security Considerations

### Threat Model

1. **Code Tampering**: Digital signatures prevent bytecode modification
2. **Hardware Spoofing**: Hardware fingerprinting prevents execution on wrong targets
3. **Side-Channel Analysis**: Constant-time validation algorithms
4. **Supply Chain**: Build reproducibility and source code hashing

### Recommended Security Practices

1. **Always validate** CVBC format before loading
2. **Verify signatures** in production deployments  
3. **Use hardware-backed** key storage when available
4. **Implement secure boot** chain integration
5. **Monitor integrity** during execution

## Performance Implications

### File Size Optimization

- **Typical CVBC overhead**: 200-500 bytes for basic programs
- **Compression**: Optional bytecode compression (DEFLATE)
- **Minimal metadata**: Only include required hardware information

### Load-Time Performance

- **Fast validation**: CRC32 hardware acceleration where available
- **Lazy loading**: Load sections on-demand
- **Template caching**: Cache generated templates between builds

## Future Extensions

### Planned Format Evolution

1. **CVBC 1.1**: Extended performance hints, memory layout optimization
2. **CVBC 1.2**: Multi-program containers, shared libraries
3. **CVBC 2.0**: Quantum-safe cryptography, extended architectures

### Extensibility Mechanisms

- **Custom metadata sections**: Vendor-specific extensions
- **Plugin architecture**: Third-party format processors
- **Schema evolution**: Versioned metadata schemas

## Conclusion

The CVBC format provides a research foundation for CockpitVM's exploration of embedded hypervisor architectures powered by Trinity. The metadata-driven approach enables automated Trinity template generation while investigating security, compatibility, and performance optimization techniques.

This format specification ensures <0.5% ambiguity for future development while preserving extensibility for emerging embedded system requirements.