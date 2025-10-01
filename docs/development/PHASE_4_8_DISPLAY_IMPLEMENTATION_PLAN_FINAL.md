# Phase 4.8: Display Implementation Plan - FINAL AUTHORITATIVE VERSION

**Document Status**: ✅ **CURRENT - USE THIS VERSION**  
**Created**: 2025-01-14  
**Context**: Post vm_compiler validation success, text-only display focus for SOS MVP  
**Authors**: Staff Embedded Systems Architect + Claude Code  

---

## **Executive Summary**

Phase 4.8 implements text-only display capabilities for the SOS (Save Our Ship) MVP on STM32G474 WeAct Studio CoreBoard. This implementation builds on the successful vm_compiler string literal system, focusing exclusively on reliable text display via I2C OLED integration with u8g2 library through the platform layer abstraction.

**Core Achievement**: Extend proven vm_compiler validation framework to include display opcodes (0x70-0x74), enabling compiled ArduinoC programs to display emergency signaling text.

---

## **Hardware Specification - CONFIRMED**

### **Display Configuration**
```yaml
Display: 128x32 SSD1306 OLED
I2C_Pins: 
  SCL: PC11  # I2C Clock
  SDA: PA8   # I2C Data  
I2C_Speed: 100kHz (standard mode)
Character_Grid: 21 cols × 4 rows (with 6x8 font)
Font: u8g2 built-in compact monospace (u8g2_font_6x8_tf)
```

### **Memory Allocation**
```yaml
Display_Buffer: 512 bytes (128×32÷8 bits)
Platform_Layer: ~2KB (u8g2 integration)
Total_Addition: ~2.5KB (fits within 24KB static allocation)
```

---

## **Implementation Architecture**

### **u8g2 Integration Pattern** (Established in COCKPITVM_INTEGRATION_ARCHITECTURE.md)
```cpp
// Platform Layer Boundary - lib/platform_stm32g4/display_driver.cpp
class DisplayDriver {
private:
    U8G2_SSD1306_128X32_NONAME_F_HW_I2C u8g2;
    
public:
    DisplayDriver() : u8g2(U8G2_R0, PC11, PA8) {}  // SCL, SDA pins
    
    void text(uint8_t col, uint8_t row, const char* str) {
        u8g2.setFont(u8g2_font_6x8_tf);
        u8g2.setCursor(col * 6, (row + 1) * 8);  // Character grid positioning
        u8g2.print(str);
        // Deferred update - no sendBuffer() here
    }
    
    void flush() {
        u8g2.sendBuffer();  // Send accumulated changes to OLED
    }
    
    void clear() {
        u8g2.clearBuffer();
    }
};

// VM Integration Boundary - C interface  
extern "C" {
    bool platform_display_text(uint8_t col, uint8_t row, const char* text);
    bool platform_display_clear();
    bool platform_display_update();
}
```

### **VM Opcode Specification (0x70-0x74)**
```c
// Phase 4.8 Display Opcodes - Text Only
OP_DISPLAY_CLEAR  = 0x70,  // Clear display buffer
OP_DISPLAY_TEXT   = 0x71,  // Draw text at grid position (stack: col, row; immediate: string_id)
OP_DISPLAY_UPDATE = 0x72,  // Flush buffer to OLED hardware  
OP_BUTTON_READ    = 0x73,  // Read button state (returns 5-bit PC0-PC4 state)
OP_LED_MORSE      = 0x74,  // Start LED morse pattern (stack: pattern_id)
```

### **Error Handling Philosophy**
- **Silent Failure**: Display opcodes return false for errors, VM execution continues
- **Bounds Checking**: Column ≥21 or Row ≥4 fails silently  
- **I2C Tolerance**: Communication failures don't halt emergency systems
- **Graceful Degradation**: System remains functional even if display fails

---

## **Chunked Implementation Plan**

### **Chunk 4.8.1: I2C Hardware Validation Foundation** 
**Duration**: 1-2 days  
**Scope**: Bare metal I2C communication verification  

#### **Tasks**:
1. **Hardware Wiring**: 128x32 OLED to PC11(SCL)/PA8(SDA) with pull-up resistors
2. **Direct I2C Test**: Raw `HAL_I2C_Master_Transmit()` calls to SSD1306
3. **Scope Validation**: Oscilloscope capture of I2C protocol compliance
4. **Protocol Verification**: Confirm SSD1306 initialization and test pattern display

#### **Expected Scope Results**:
```c
I2C_ADDRESS: 0x3C (7-bit) / 0x78 (8-bit write)
CLOCK_FREQ: ~100kHz on PC11 (SCL) 
DATA_PROTOCOL: Valid start/stop/ack sequences on PA8 (SDA)
INIT_SEQUENCE: SSD1306 wakeup + display mode configuration
TEST_PATTERN: Pixel data successfully transmitted and displayed
```

#### **Exit Criteria**: Clean I2C signals on oscilloscope, OLED displays test pattern via direct HAL calls

---

### **Chunk 4.8.2: Platform Layer u8g2 Integration**
**Duration**: 1-2 days  
**Scope**: u8g2 library integration with platform abstraction  

#### **Tasks**:
1. **PlatformIO Setup**: Add `olikraus/U8g2@^2.35.9` to platformio.ini dependencies
2. **u8g2 Wrapper**: Implement `DisplayDriver` class in `lib/platform_stm32g4/`
3. **C Interface**: Create `platform_display_*()` functions for VM boundary crossing
4. **Deferred Buffer**: Implement accumulate-and-flush pattern (`clear()` → `text()` → `flush()`)
5. **Basic Testing**: "Hello World" text display through platform abstraction

#### **Implementation Details**:
```cpp
// lib/platform_stm32g4/display_driver.cpp
void DisplayDriver::init() {
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_6x8_tf);  // Compact monospace
    u8g2.clearDisplay();
}

bool DisplayDriver::text(uint8_t col, uint8_t row, const char* str) {
    if (col >= 21 || row >= 4) return false;  // Silent bounds check
    u8g2.setCursor(col * 6, (row + 1) * 8);  // Convert grid to pixels
    u8g2.print(str);
    return true;  // Deferred - no sendBuffer() yet
}
```

#### **Exit Criteria**: Multi-line text successfully displayed via platform layer with deferred updates

---

### **Chunk 4.8.3: VM Opcode Implementation**
**Duration**: 2-3 days  
**Scope**: Display opcodes (0x70-0x74) integrated into ExecutionEngine  

#### **Tasks**:
1. **Opcode Registration**: Add display opcodes to `vm_opcodes.h` and `is_opcode_implemented()`
2. **Handler Implementation**: Display opcode switch cases in `ExecutionEngine::execute_single_instruction()`
3. **String Integration**: Connect to existing string literal system from enhanced bytecode format
4. **IOController Enhancement**: Add display context management to maintain state
5. **Stack Management**: Proper parameter extraction (col, row, string_id)

#### **Handler Implementation**:
```cpp
// ExecutionEngine::execute_single_instruction()
case VMOpcode::OP_DISPLAY_TEXT: {
    uint8_t string_id = static_cast<uint8_t>(immediate);
    int32_t row, col;
    if (!pop(row) || !pop(col)) return false;
    
    if (!is_valid_string_id(string_id)) return false;
    const char* text = get_string_literal(string_id);
    
    return io.display_text(static_cast<uint8_t>(col), 
                          static_cast<uint8_t>(row), text);
}

case VMOpcode::OP_DISPLAY_CLEAR: {
    return io.display_clear();
}

case VMOpcode::OP_DISPLAY_UPDATE: {
    return io.display_flush();  // Calls u8g2.sendBuffer()
}
```

#### **Exit Criteria**: VM executes display opcodes, text appears in buffer, flush command updates OLED

---

### **Chunk 4.8.4: Compiler Integration & Testing**
**Duration**: 2-3 days  
**Scope**: ArduinoC compiler support + comprehensive validation testing  

#### **Tasks**:
1. **BytecodeVisitor Enhancement**: Add display function recognition (`display_clear()`, `display_text()`, `display_update()`)
2. **Function Mapping**: Map ArduinoC functions to display opcodes with proper parameter handling  
3. **Test Suite Creation**: Display validation tests building on string literal success framework
4. **Integration Testing**: Complete compile→execute→display pipeline validation
5. **SOS Demonstration**: Multi-line emergency text display program

#### **Compiler Integration**:
```cpp
// BytecodeVisitor - function call handling
if (function_name == "display_clear") {
    emitInstruction(0x70, 0, 0);  // OP_DISPLAY_CLEAR
} else if (function_name == "display_text") {
    // Args: col, row, string_literal  
    // Stack: [col] [row], immediate: string_id
    emitInstruction(0x71, 0, string_id);  // OP_DISPLAY_TEXT
} else if (function_name == "display_update") {
    emitInstruction(0x72, 0, 0);  // OP_DISPLAY_UPDATE  
}
```

#### **Test Suite Structure**:
```c
// test_display_basic.c
void setup() {
    display_clear();
    display_text(0, 0, "Test Line 1");
    display_text(0, 1, "Test Line 2"); 
    display_update();
}

// test_display_sos.c  
void setup() {
    display_clear();
    display_text(0, 0, "SOS EMERGENCY");
    display_text(0, 1, "GPS: 40.7,-74.0");
    display_text(0, 2, "Battery: 67%");
    display_text(0, 3, "Press * for help");
    display_update();
}
```

#### **Exit Criteria**: 100% test pass rate for display opcodes, working SOS emergency text display program

---

## **Success Metrics & Validation**

### **Per-Chunk Validation**
- **4.8.1**: Oscilloscope shows clean 100kHz I2C, SSD1306 initialization successful
- **4.8.2**: "Hello World" via u8g2 platform wrapper, deferred updates working
- **4.8.3**: VM display opcodes execute, text accumulates and flushes correctly  
- **4.8.4**: Compiled ArduinoC displays 4-line SOS emergency text

### **Integration Success Criteria**
```yaml
Compiler_Success: ArduinoC → bytecode → VM execution → OLED display
String_Integration: Display text uses existing string literal loading system
Error_Tolerance: Silent failures don't crash emergency systems
Memory_Budget: Total addition ≤3KB within 24KB static allocation
Test_Coverage: 100% pass rate extending proven vm_compiler validation framework
```

---

## **Design Decisions & Context**

### **Text-Only Rational** 
**Decision**: Phase 4.8 implements only text display opcodes (0x70-0x74), reserving graphics primitives for Phase 5.0
**Reasoning**: 
- SOS MVP requires reliable text display for emergency information  
- Text-only reduces complexity and memory footprint
- Builds incrementally on successful string literal implementation
- Establishes solid foundation for future graphics expansion

### **u8g2 Integration Strategy**
**Decision**: Use u8g2 library through platform layer abstraction, never exposed to VM or user code
**Reasoning**: 
- Leverages proven, mature OLED library with STM32 I2C support
- Platform layer abstraction maintains clean architectural boundaries
- u8g2 complexity isolated from hypervisor core
- Enables future display hardware changes without VM modifications

### **Deferred Update Pattern**
**Decision**: Accumulate display changes, flush only on `OP_DISPLAY_UPDATE` 
**Reasoning**:
- Reduces I2C traffic for multi-line displays
- Allows atomic display updates (prevents partial text during updates)
- Mirrors u8g2's natural buffer-then-send pattern
- Better performance for complex display layouts

### **Silent Error Handling**
**Decision**: Display failures return false but don't halt VM execution
**Reasoning**:
- Emergency systems must remain functional even if display fails
- I2C communication can be unreliable in harsh environments  
- Silent failure allows graceful degradation of non-critical features
- Consistent with embedded safety system design principles

---

## **Future Phase 5.0 Extensions**

### **Reserved Opcode Ranges**
```c
// Graphics Primitives (0x80-0x8F) - Future Phase 5.0
OP_DRAW_PIXEL     = 0x80,  // Set/clear individual pixel
OP_DRAW_LINE      = 0x81,  // Line between two points
OP_DRAW_RECT      = 0x84,  // Rectangle outline
OP_FILL_RECT      = 0x85,  // Filled rectangle
OP_DRAW_CIRCLE    = 0x86,  // Circle outline
OP_FILL_CIRCLE    = 0x87,  // Filled circle

// UI Components (0x90-0x9F) - Future Phase 5.0  
OP_DIALOG_SHOW    = 0x90,  // Static dialog display
OP_MENU_NAVIGATE  = 0x91,  // Static menu system
OP_WIDGET_REFRESH = 0x92,  // Update predefined widget
```

### **Evolution Path**
Phase 4.8 text-only implementation provides the architectural foundation for rich graphics in Phase 5.0, including bitmap operations, primitive drawing, and structured UI components with static allocation.

---

## **Implementation Timeline**

**Estimated Duration**: 6-8 days total  
**Methodology**: Sequential chunks, hardware-first validation, TDD 100% pass rate  
**Dependencies**: Successful vm_compiler string literal system (✅ completed)
**Parallel Work**: Phase 4.9 Unified Handler Architecture planning can proceed

### **Critical Path**:
1. **Day 1-2**: Hardware I2C validation with oscilloscope  
2. **Day 3-4**: u8g2 platform layer integration
3. **Day 5-6**: VM opcode implementation  
4. **Day 7-8**: Compiler integration + comprehensive testing

**Ready for Implementation**: All ambiguity resolved, hardware specification confirmed, architectural patterns established.

---

## **Context Preservation**

This document captures the complete context from our implementation planning session:

- **vm_compiler Success**: 100% test pass rate with string literal enhancement provides proven foundation
- **Architecture Alignment**: Uses established u8g2 integration pattern from COCKPITVM_INTEGRATION_ARCHITECTURE.md
- **Hardware Constraints**: STM32G474 WeAct Studio CoreBoard, 24KB static allocation, emergency system requirements
- **Methodological Approach**: Hardware-first validation, chunked implementation, platform layer abstraction
- **Design Philosophy**: KISS+Evolution, text-only for reliability, silent failure for safety systems

All implementation guidance, hardware specifications, and architectural decisions are preserved for future development continuation.