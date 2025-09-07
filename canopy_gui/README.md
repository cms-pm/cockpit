# Canopy - CockpitVM Bytecode Uploader GUI

**Project**: C++ Qt6 GUI application for CockpitVM bytecode uploading  
**Architecture**: C++ with RAII resource management + Oracle library integration  
**Target**: Embedded developers requiring bytecode deployment tools

## Project Overview

Canopy provides a GUI interface for uploading bytecode to CockpitVM hardware targets. Built using the Oracle bootloader client components, it focuses on reliable bytecode deployment with clear user feedback.

### Key Features
- Hardware discovery and CockpitVM device identification
- Bytecode upload with progress indication
- Protocol diagnostics for troubleshooting
- Extensible architecture for future hardware support

### Architecture Design
- **RAII Resource Management**: Automatic cleanup of serial connections
- **Oracle Library Integration**: Reuse of existing frame parser and protocol components
- **Exception Safety**: Structured error handling
- **Single Executable**: Minimal dependencies

## Directory Structure

```
canopy_gui/
├── src/                    # C++ source files
│   ├── main/              # Application entry point
│   ├── gui/               # Qt6 GUI components  
│   ├── protocol/          # Protocol integration layer
│   └── hardware/          # Hardware discovery and management
├── include/               # Public header files
├── shared_libs/          # Oracle library integration
│   ├── frame_parser/     # Universal frame parser from Oracle
│   └── protocol_client/  # Protocol client components
├── resources/            # Qt resources (UI files, icons, etc.)
├── tests/                # Unit and integration tests
├── docs/                 # Project documentation
└── build/                # Build artifacts (gitignored)
```

## Development Phases

### Phase CANOPY-1: Foundation (C++ + Qt6)
- CANOPY-1.1: Build system and Oracle library integration
- CANOPY-1.2: Core protocol client with RAII resource management  
- CANOPY-1.3: Basic Qt6 GUI framework
- CANOPY-1.4: Threading architecture with exception safety

### Phase CANOPY-2: Hardware & Communication
- CANOPY-2.1: Device discovery and enumeration
- CANOPY-2.2: CockpitVM device identification via handshake
- CANOPY-2.3: Connection management with error handling

### Phase CANOPY-3: Upload Workflow
- CANOPY-3.1: Bytecode file validation and parsing
- CANOPY-3.2: Upload progress indication and cancellation
- CANOPY-3.3: Result reporting with error guidance

### Phase CANOPY-4: Advanced Features
- CANOPY-4.1: Advanced diagnostics panel
- CANOPY-4.2: Protocol frame analysis display
- CANOPY-4.3: Extensibility framework

## Build Requirements

- **Qt6**: Core, Widgets, SerialPort modules
- **CMake**: 3.20+ for Qt6 integration
- **C++20**: For modern language features
- **Compiler**: GCC 11+ or Clang 13+ with C++20 support

## Oracle Library Dependencies

Canopy integrates with existing Oracle components:
- `tests/oracle_bootloader/lib/frame_parser_universal.hpp` - Frame parsing
- Protocol definitions from bootloader specification
- CRC validation and error handling patterns

## Getting Started

```bash
cd canopy_gui
mkdir build && cd build  
cmake ..
make -j$(nproc)
./canopy
```

---

**Project Status**: Phase CANOPY-1 Planning  
**Implementation Language**: C++ with Qt6  
**Integration Strategy**: Oracle library reuse with RAII patterns