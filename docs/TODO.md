# 📋 Cockpit Project TODO List

> **Current Status**: Phase 2 Complete - Ready for Phase 3 C Compiler Development

## 🔥 High Priority Items (Active Development)

### **Phase 3: C Compiler Implementation (CURRENT)**
- **Phase 3, Chunk 3.1: Minimal C Parser Foundation (8 hours)** - ANTLR grammar + C++ visitor pattern
- **Phase 3, Chunk 3.2: Essential Control Flow (6 hours)** - if/else, while with jump resolution
- **Phase 3, Chunk 3.3: Basic Functions (6 hours)** - User-defined functions with parameters

### **Phase 3 Architecture Decisions (FINALIZED)**
- ✅ **ANTLR Grammar**: Incremental complexity per chunk
- ✅ **C++ Implementation**: Leverage OOP patterns for clean architecture
- ✅ **Linear Symbol Table**: Simple O(n) lookup for Arduino-scale programs
- ✅ **Visitor Pattern**: Clean separation of parsing and code generation
- ✅ **Two-Pass Compilation**: Robust jump resolution with backpatching

## 🎯 Medium Priority Items (Future Phases)

### **Platform Expansion**
- **Add Cortex M0/M0+ support** - Resource-constrained device targeting for cost-effective deployments
- **Implement RTOS pre-emptive scheduling** - Real-time application support with deterministic timing
- **Add DMA controller integration** - High-performance data transfer capabilities for advanced peripherals

### **Phase 3 Expansion Options (Deferred)**
- **Hash-Based Symbol Tables** - O(1) lookup performance for large programs
- **Advanced Scope Management** - Nested block scoping with scope stacks
- **Template-Based Code Generation** - Pattern-matching bytecode templates
- **Comprehensive Debug Infrastructure** - Professional-grade compilation debugging
- **Extended VM Memory Layout** - Global/local/call stack separation

### **Phase 4 Features (Deferred from Phase 3)**
- **Array Support** - Array declarations, indexing, and bounds checking
- **Complex Expressions** - Operator precedence, ternary operators, complex arithmetic
- **for Loops and break/continue** - Enhanced control flow constructs
- **Multiple Data Types** - char, unsigned long, pointers with type checking
- **Global Variable Initialization** - Static initialization of global variables
- **Advanced Function Features** - Function overloading, default parameters, recursion

### **Language Support (Future)**
- **Add Rust bytecode support** - Safe systems programming with memory safety guarantees and zero-cost abstractions

### **Infrastructure**
- **Implement CI/CD pipeline** - Automated testing and deployment infrastructure (foundation ready)

### **Hardware-In-Loop/Software-In-Loop (HIL/SIL) Testing (Post-MVP)**
- **Implement GPIO injection simulation** - Enhanced QEMU GPIO state injection for pullup behavior testing
- **Add hardware-specific analog testing** - Real ADC behavior validation with varying input ranges
- **Implement button debouncing validation** - Hardware pullup resistor behavior and timing verification  
- **Add precise timing validation** - Cycle-accurate timing tests for real-time applications
- **Create HIL test framework** - Real hardware integration via USB-Serial bridge for critical path validation
- **Implement SIL shadow state tracking** - Parallel GPIO state tracker for expected vs actual hardware behavior
- **Implement behavioral models for analog simulation** - Realistic ADC/DAC simulation with noise, linearity, and timing characteristics
- **Add QEMU_CLOCK_VIRTUAL integration** - Virtual time synchronization for behavioral models and event timing
- **Create mock button physical simulation** - Switch bounce, contact resistance, and mechanical timing models
- **Integrate QEMU Clock API for analog behavior simulation** - Replace simple virtual counter with proper QEMU time domains for realistic analog signal timing, ADC conversion periods, and sensor response modeling

## 🔵 Low Priority Items (Polish & Enhancement)

### **Testing Framework**
- **Implement hybrid testing** - Unit + integration test framework after basic structure validation

## ✅ Completed Items (Reference)

### **Project Foundation (Phase 1)**
- ✅ **Phase 1, Chunk 1.1: Project Structure Setup** - PlatformIO + QEMU integration
- ✅ **Phase 1, Chunk 1.2: VM Core Stack Operations** - 8 opcodes, comprehensive testing  
- ✅ **Phase 1, Chunk 1.3: QEMU Integration Foundation** - Semihosting, automation scripts

### **Arduino Integration (Phase 2)**
- ✅ **Phase 2, Chunk 2.1: Arduino Digital GPIO Foundation** - Arduino HAL with VM integration
- ✅ **Phase 2, Chunk 2.2: Arduino Input + Button** - Debouncing, event queue, button opcodes
- ✅ **Phase 2, Chunk 2.3.1: pinMode + Timing Functions** - pinMode, millis, micros opcodes
- ✅ **Phase 2, Chunk 2.3.2: printf() with Semihosting** - %d %s %x %c format support
- ✅ **Phase 2, Chunk 2.3.3: Comparison Operations** - EQ/NE/LT/GT/LE/GE opcodes
- ✅ **Phase 2, Chunk 2.3.4: C-to-Bytecode Examples** - Phase 3 preparation and integration tests
- ✅ **Phase 2, Chunk 2.3.5: Documentation + Architecture Validation** - Complete Phase 2 validation
- ✅ **Implement QEMU GPIO pullup test solution** - Hardware simulation compatibility
- ✅ **Fix VM core test framework accounting bug** - Memory corruption resolved

### **Project Management**
- ✅ **Merge all development branches into main branch** - Clean repository structure
- ✅ **Update README to separate current vs planned features** - Clear scope documentation
- ✅ **Add visual enhancements and new planned features to README** - Professional presentation

### **Planning & Architecture** 
- ✅ **Analyze compiler implementation approaches for MVP/PoC** - Hand-written minimal parser selected
- ✅ **Finalize implementation roadmap based on user answers** - 5-week timeline established
- ✅ **Create focused development plan for minimal parser approach** - KISS principle applied
- ✅ **Pool final implementation questions for development plan** - Requirements clarified
- ✅ **Analyze function mapping and bytecode format strategies** - 16-bit instruction format
- ✅ **Address final development plan questions before implementation** - Decisions confirmed
- ✅ **Create graduated implementation plan with mitigation strategies** - Risk management
- ✅ **Compose comprehensive implementation plan with manageable chunks** - Phased approach
- ✅ **Await final pre-implementation decisions** - Ready for development
- ✅ **Run final Phase 2.1 validation tests** - 89% success rate achieved

## 📊 Priority Classification

### **High Priority** (Active Development)
Focus on immediate Phase 2 completion and core functionality.

### **Medium Priority** (Next Phase Planning)
Platform expansion, language support, and infrastructure improvements.

### **Low Priority** (Future Enhancement)
Testing framework improvements and quality-of-life features.

## 🎯 Next Milestone

**Immediate Focus**: Complete Phase 2, Chunk 2.2 (Arduino Input + Button handling)

**Success Criteria**: 
- Enhanced button debouncing implementation
- Interrupt-based input processing
- Comprehensive input validation testing
- Documentation of input handling architecture

---

*Last Updated: Current session - Phase 2.1 Complete*
*Total Items: 26 (19 completed, 7 pending)*
*Success Rate: 73% completion*