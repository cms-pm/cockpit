# VM_Compiler

**C to ComponentVM Bytecode Compiler Library with Development Tools**  
**Version**: 3.10.0

## Overview

This PlatformIO library provides the ComponentVM C-to-bytecode compiler with integrated development tools for grammar development, validation, and testing.

## Library Usage

```cpp
#include <vm_compiler.h>

// Use compiler functionality in embedded projects
```

## Development Tools

### Grammar Generation
```bash
cd lib/vm_compiler/grammar
./generate.sh
```

### Development Build
```bash
cd lib/vm_compiler/tools
./build_dev_tools.sh
```

### Validation Suite
```bash
cd lib/vm_compiler/tools
./run_validation.sh
```

## Structure

- `src/` - Core compiler source code
- `grammar/` - ANTLR4 grammar and generation tools
- `validation/` - Validation tests (excluded from library builds)
  - `compiler/` - Compiler validation tests
  - `integration/` - Integration tests
  - `grammar/` - Grammar validation
- `development/` - Development testing (excluded from library builds)
  - `debug/` - Debug and diagnostic tests
  - `experiments/` - Experimental features
- `tools/` - Development scripts and build system

## Dependencies

- **ComponentVM**: Core VM library
- **ANTLR4 Runtime**: For development tools only (not included in embedded builds)

## License

MIT