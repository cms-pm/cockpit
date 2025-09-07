# Phase CANOPY-1: Foundation Implementation Plan

**Duration**: 2-3 days  
**Architecture**: C++ Qt6 with Oracle library integration  
**Validation**: User workflow acceptance criteria

---

## Architecture Refinements for C++

### **Shared Library Integration Strategy**
**Decision**: Protocol-level boundary (not operation-level)
- Expose individual protocol operations: `handshake()`, `prepare_flash()`, `send_data_packet()`, `verify_flash()`
- Maintain visibility into protocol execution for debugging
- Enable fine-grained error recovery and retry logic

### **Resource Management Pattern**
```cpp
// RAII pattern for serial connections
class SerialConnection {
    std::unique_ptr<QSerialPort> port_;
public:
    SerialConnection(const QString& device);
    ~SerialConnection() = default;  // Automatic cleanup
    
    // Move-only semantics
    SerialConnection(SerialConnection&&) = default;
    SerialConnection& operator=(SerialConnection&&) = default;
};
```

### **Error Handling Architecture**
```cpp
// Exception-safe error propagation
template<typename T>
using Result = std::expected<T, ProtocolError>;

Result<HandshakeResponse> performHandshake(const QString& device);
Result<void> uploadBytecode(const std::vector<uint8_t>& data);
```

---

## CANOPY-1.1: Build System & Oracle Integration (4 hours)

### **CANOPY-1.1.1: CMake Build System (2 hours)**
- Modern CMake with Qt6 integration
- Oracle library inclusion from `tests/oracle_bootloader/lib/`
- Cross-platform build configuration
- **Done Criteria**: `cmake .. && make` builds successfully

### **CANOPY-1.1.2: Oracle Library Integration (2 hours)**  
- Copy and adapt `frame_parser_universal.hpp` to `shared_libs/`
- Create C++ wrapper for Oracle protocol client functionality
- Integrate CRC validation and frame parsing components
- **Done Criteria**: Can parse frames using existing Oracle code

---

## CANOPY-1.2: Protocol Client with RAII (6 hours)

### **CANOPY-1.2.1: Serial Connection Management (2 hours)**
- RAII wrapper around QSerialPort
- Automatic resource cleanup on destruction
- Thread-safe connection state management
- **Done Criteria**: Serial ports properly opened/closed without leaks

### **CANOPY-1.2.2: Protocol Operations (3 hours)**
- Individual protocol functions (handshake, prepare, transfer, verify)
- Exception-safe error handling with std::expected
- Integration with Oracle frame parser
- **Done Criteria**: Can execute complete protocol sequence

### **CANOPY-1.2.3: Error Recovery Framework (1 hour)**
- Structured error types for different failure modes
- Retry logic for transient errors (CRC failures, timeouts)
- Clear error propagation to GUI layer
- **Done Criteria**: Predictable error handling behavior

---

## CANOPY-1.3: Qt6 GUI Framework (5 hours)

### **CANOPY-1.3.1: Main Window Structure (2 hours)**
- Basic QMainWindow with menu bar and status bar
- File selection dialog integration
- Device selection dropdown
- Upload button with enable/disable states
- **Done Criteria**: GUI opens with functional file selection

### **CANOPY-1.3.2: UI Layout Design (2 hours)**
- Simple tab widget (Simple/Advanced tabs)
- Progress bar for upload indication  
- Text area for status messages
- **Done Criteria**: Tab switching works, UI elements respond

### **CANOPY-1.3.3: Qt Resource Integration (1 hour)**
- .ui files for complex dialogs
- Application icons and resources
- Resource compilation into executable
- **Done Criteria**: Resources load correctly, professional appearance

---

## CANOPY-1.4: Threading Architecture (5 hours)

### **CANOPY-1.4.1: QThread Worker Pattern (3 hours)**
- ProtocolWorker class inheriting QObject
- Signal/slot communication with main thread
- Thread-safe progress reporting
- **Done Criteria**: Protocol operations don't block GUI

### **CANOPY-1.4.2: Exception Safety Across Threads (1 hour)**  
- Exception handling in worker thread
- Error signal emission to main thread
- Resource cleanup on thread termination
- **Done Criteria**: No crashes on protocol errors

### **CANOPY-1.4.3: Cancellation Support (1 hour)**
- Atomic cancellation flag for worker thread
- Graceful protocol operation termination
- UI state restoration after cancellation
- **Done Criteria**: Upload cancellation works reliably

---

## Validation Framework

Each chunk validates against **User Acceptance Criteria**:

1. **Functional Requirements**: Core functionality works as specified
2. **Resource Management**: No leaked resources (serial ports, memory, threads)
3. **Error Handling**: Clear error messages with appropriate recovery options
4. **Code Quality**: Clean, maintainable C++ with proper RAII patterns

## Oracle Library Reuse Strategy

### **Direct Integration Points**
- `frame_parser_universal.hpp` - Binary frame parsing with CRC validation
- Protocol constants and message definitions
- Error handling patterns and recovery strategies

### **Adaptation Requirements**
- Qt integration for serial I/O (QSerialPort instead of Python serial)
- Signal/slot integration for progress reporting
- Exception handling instead of Python error returns

---

## Risk Mitigation

### **Technical Risks**
- **Qt6 Learning Curve**: Mitigated by focusing on core widgets first
- **Oracle Integration Complexity**: Mitigated by starting with existing C++ reference
- **Threading Synchronization**: Mitigated by using proven Qt patterns

### **Schedule Risks**  
- **C++ Setup Overhead**: +2 hours for CMake/Qt configuration
- **Oracle Adaptation**: -4 hours saved by reusing existing C++ code
- **Net Impact**: 2 hours savings vs Python approach

---

**Phase CANOPY-1 Total Effort**: 20 hours  
**Key Deliverable**: Functional C++ GUI that can execute complete bytecode upload using Oracle protocol components  
**Success Criteria**: User can select file, choose device, upload bytecode with progress feedback

---

**Next Phase**: CANOPY-2 Hardware Discovery & Communication