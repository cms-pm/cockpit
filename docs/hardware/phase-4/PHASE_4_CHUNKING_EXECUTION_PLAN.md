# Phase 4 Chunking Execution Plan: QEMU to STM32G431CB Hardware Transition

**Version**: 1.0 | **Date**: July 11, 2025 | **Target**: STM32G431CB WeAct Studio CoreBoard  
**Status**: Ready for execution | **Total Duration**: 19-24 hours across 10 systematic chunks

---

## Phase 4.1: Hardware Foundation (4-5 hours)

### **Chunk 4.1.1: PlatformIO Environment & Minimal Validation**
- **Duration**: 2 hours
- **Dependencies**: Hardware procurement complete
- **Objective**: Activate STM32G431CB environment and validate basic hardware communication
- **Deliverables**:
  - Uncomment and configure `[env:weact_g431cb_hardware]` in `platformio.ini`
  - Create minimal LED blink test program
  - Establish SWD programming pipeline with ST-Link
  - Validate hardware-software toolchain integration
- **Success Criteria**:
  - LED blinks at 1Hz on hardware
  - SWD programming succeeds consistently
  - PlatformIO hardware environment functional

### **Chunk 4.1.2: Hardware Abstraction Layer Adaptation**
- **Duration**: 2-3 hours
- **Dependencies**: 4.1.1 complete
- **Objective**: Port existing Arduino HAL to STM32G4 hardware specifics
- **Deliverables**:
  - Adapt `lib/arduino_hal/` for STM32G4 register access
  - Implement GPIO, timing, and ADC functions for real hardware
  - Create hardware-specific performance benchmarks
  - Validate Arduino API compatibility on target hardware
- **Success Criteria**:
  - All Arduino API functions operational on hardware
  - Timing precision within 5% of QEMU baseline
  - GPIO operations complete within 100μs

---

## Phase 4.2: VM-Hardware Integration (4-5 hours)

### **Chunk 4.2.1: Core VM Hardware Integration**
- **Duration**: 2-3 hours
- **Dependencies**: 4.1.2 complete
- **Objective**: Get ComponentVM executing on hardware with hardcoded bytecode
- **Deliverables**:
  - Integrate VM library with STM32G4 HAL
  - Embed test bytecode programs in Flash memory
  - Validate basic instruction execution on hardware
  - Establish memory protection mechanisms
- **Success Criteria**:
  - VM executes hardcoded bytecode on hardware
  - Basic arithmetic and stack operations functional
  - Memory protection mechanisms active

### **Chunk 4.2.2: Complete System Integration**
- **Duration**: 2-3 hours
- **Dependencies**: 4.2.1 complete
- **Objective**: Full system functionality with debugging capabilities
- **Deliverables**:
  - Implement printf debugging via UART/semihosting
  - Complete Arduino API hardware integration
  - Add interrupt handling and system initialization
  - Validate all VM components on hardware
- **Success Criteria**:
  - Complete Arduino API functional on hardware
  - Printf output working via UART/semihosting
  - System interrupts and timing operational

---

## Phase 4.3: Automated Testing Infrastructure (3-4 hours)

### **Chunk 4.3.1: SWD-Based Test Framework**
- **Duration**: 2-3 hours
- **Dependencies**: 4.2.2 complete
- **Objective**: Enable Claude-accessible automated hardware testing
- **Deliverables**:
  - Create automated test runner using OpenOCD/SWD
  - Implement semihosting test result capture
  - Build hardware test automation scripts
  - Establish continuous hardware validation pipeline
- **Success Criteria**:
  - Automated test execution via SWD successful
  - Test results captured and parsed automatically
  - Claude can execute hardware tests and interpret results

### **Chunk 4.3.2: Hardware vs QEMU Validation**
- **Duration**: 1-2 hours
- **Dependencies**: 4.3.1 complete
- **Objective**: Comprehensive platform comparison and benchmarking
- **Deliverables**:
  - Run all 181 tests on hardware platform
  - Performance analysis vs QEMU baseline
  - Document hardware-specific behaviors and limitations
  - Validate production readiness
- **Success Criteria**:
  - All 181 VM tests pass on hardware
  - Performance within 10% of QEMU baseline
  - No critical hardware-specific issues identified

---

## Phase 4.4: Custom Bootloader System (5-6 hours)

### **Chunk 4.4.1: Bootloader Foundation**
- **Duration**: 2-3 hours
- **Dependencies**: 4.3.2 complete
- **Objective**: Basic bootloader with UART communication interface
- **Deliverables**:
  - Implement minimal bootloader (16KB Flash partition)
  - Create UART communication interface
  - Add DTR trigger detection for update mode
  - Establish memory partitioning system
- **Memory Layout**: 
  - Bootloader: 16KB (0x08000000-0x08004000)
  - Hypervisor: 48KB (0x08004000-0x08010000)
  - Bytecode: 64KB (0x08010000-0x08020000)
- **Success Criteria**:
  - Bootloader boots and initializes correctly
  - UART communication functional
  - DTR trigger detection working

### **Chunk 4.4.2: UART Protocol Implementation**
- **Duration**: 2-3 hours
- **Dependencies**: 4.4.1 complete
- **Objective**: Robust packet communication protocol
- **Deliverables**:
  - Implement packet parsing with CRC16 validation
  - Create command-response protocol
  - Add error handling and recovery mechanisms
  - Validate communication reliability
- **Protocol Format**: `[SYNC][TYPE][LEN][PAYLOAD][CRC16]`
- **Success Criteria**:
  - Packet reception and parsing functional
  - CRC16 validation working correctly
  - Command-response protocol operational

### **Chunk 4.4.3: Flash Programming System**
- **Duration**: 1-2 hours
- **Dependencies**: 4.4.2 complete
- **Objective**: Safe Flash erase/write operations
- **Deliverables**:
  - Implement STM32G4 Flash programming operations
  - Add data verification and integrity checking
  - Create Flash operation safety mechanisms
  - Validate programming reliability
- **Success Criteria**:
  - Flash erase operations successful
  - Flash programming working correctly
  - Data verification functional

---

## Phase 4.5: Production Tools & Integration (3-4 hours)

### **Chunk 4.5.1: Host-Side Upload Tool**
- **Duration**: 2-3 hours
- **Dependencies**: 4.4.3 complete
- **Objective**: Complete bytecode deployment system
- **Deliverables**:
  - Create Python bytecode upload tool
  - Implement serial communication with bootloader
  - Add upload verification and error handling
  - Create user-friendly deployment interface
- **Usage**: `python tools/bytecode_uploader.py --port /dev/ttyUSB0 --bytecode program.bin --verify`
- **Success Criteria**:
  - Bytecode upload via UART successful
  - Upload tool handles errors gracefully
  - Verification process working correctly

### **Chunk 4.5.2: End-to-End Integration**
- **Duration**: 1-2 hours
- **Dependencies**: 4.5.1 complete
- **Objective**: Complete production workflow validation
- **Deliverables**:
  - Validate complete C→bytecode→upload→execute workflow
  - Test SOS morse code demo on hardware
  - Final performance characterization
  - Production deployment documentation
- **Success Criteria**:
  - Complete C→bytecode→hardware workflow functional
  - SOS morse code demo working on hardware
  - All performance targets met

---

## Sequential Dependencies & Validation Strategy

### **Per-Chunk Requirements**
- **Hardware validation**: Real hardware testing after implementation
- **Automated testing**: SWD-based test execution where applicable
- **Performance benchmarking**: Hardware vs QEMU comparison
- **Documentation updates**: Progressive context optimization

### **Continuous Integration**
- **Pre-chunk validation**: Automated test execution before each chunk
- **Post-implementation testing**: Comprehensive validation after each chunk
- **Regression testing**: Full test suite execution with major changes
- **Performance monitoring**: Continuous baseline tracking

### **Quality Gates**
- All tests must pass before proceeding to next chunk
- Hardware validation required for each chunk
- Performance within acceptable thresholds
- Documentation updated with each completion

---

## Success Metrics

### **Technical Targets**
- **Test Success Rate**: All 181 tests pass on hardware
- **Performance**: Within 10% of QEMU baseline (adjusted for clock speed)
- **Memory Usage**: Within 32KB RAM constraints
- **Reliability**: Bootloader and upload system operational

### **Production Readiness**
- **Complete Workflow**: C→bytecode→upload→execute functional
- **Demo Program**: SOS morse code working on hardware
- **Deployment Tools**: User-friendly upload interface
- **Documentation**: Complete production procedures

### **Development Efficiency**
- **Automated Testing**: Claude-accessible hardware validation
- **Clear Dependencies**: Sequential chunk progression
- **Context Management**: Optimized CLAUDE.md throughout
- **Quality Assurance**: Comprehensive validation at each step

---

**Next Action**: Begin Chunk 4.1.1 - PlatformIO Environment & Minimal Validation