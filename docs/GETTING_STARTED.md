# Getting Started with ComponentVM

**Quick Start Guide | 5-Minute Overview**  
**Version**: 3.10.0 | **Target**: New Developers and Evaluators

---

## 🎯 What is ComponentVM?

ComponentVM is a **embedded hypervisor** that runs C bytecode programs on ARM Cortex-M4 microcontrollers. It provides:

- **Memory-safe execution** with stack canaries and bounds checking
- **Arduino-compatible API** for familiar embedded development
- **C-to-bytecode compilation** with full toolchain integration
- **Hardware abstraction** enabling portable embedded applications

**Target Use Cases**: IoT devices, embedded control systems, safety-critical applications, educational platforms

**Currently under heavy development - things will break - not for production use**

---

---

## 📋 Architecture at a Glance

### **System Components**
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────┐
│ ExecutionEngine │◄──►│ MemoryManager    │◄──►│IOController │
│                 │    │                  │    │             │
│ • PC Management │    │ • Stack (12KB)   │    │ • Arduino   │
│ • Instruction   │    │ • Heap (10KB)    │    │   API       │
│   Dispatch      │    │ • Globals (2KB)  │    │ • Printf    │
│ • Error Prop.   │    │ • Memory Safety  │    │ • Hardware  │
└─────────────────┘    └──────────────────┘    └─────────────┘
```

### **Development Workflow**
```
C Source → ANTLR4 Compiler → Bytecode → Embedded Firmware → Hardware
```

### **Memory Layout (STM32G431CB)**
```
Flash (128KB): [Firmware 96KB][Bytecode 30KB][Config 2KB]
RAM (32KB):    [System 8KB][VM Memory 24KB]
```

---

## 🎯 Current Status & Next Steps

### **✅ Phase 3 Complete**
- Complete VM architecture with 32-bit instruction set
- Full Arduino API integration (GPIO, timing, printf)
- Comprehensive memory protection and testing
- 181/181 tests passing (100% success rate)

### **🚀 Phase 4: Hardware Transition**
- **Target**: STM32G431CB WeAct Studio CoreBoard
- **Goal**: Hardware validation and custom bootloader
- **Timeline**: 5 phases, 13-15 hours estimated

### **🔄 Your Next Steps**
1. **Explore Architecture**: [Architecture Documentation](architecture/)
2. **Review API**: [Complete API Reference](API_REFERENCE_COMPLETE.md)
3. **Hardware Planning**: [Hardware Integration Guide](hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)
4. **Join Development**: See current [TODO.md](TODO.md)

---

## 📚 Key Documentation

### **Essential Reading**
- **[Project Vision](VISION.md)** - Strategic direction and goals
- **[Architecture Overview](architecture/01-system-overview.md)** - System design philosophy
- **[API Reference](API_REFERENCE_COMPLETE.md)** - Complete function documentation

### **Technical Deep Dives**
- **[Memory Architecture](architecture/02-memory-instruction-architecture.md)** - Memory layout and instruction format
- **[Component Integration](architecture/03-component-integration-flows.md)** - Inter-component communication
- **[Build System](architecture/04-build-system-workflow.md)** - Development pipeline

### **Hardware & Implementation**
- **[Hardware Integration](hardware/integration/HARDWARE_INTEGRATION_GUIDE.md)** - STM32G431CB comprehensive guide
- **[Development Journey](development/)** - Phase implementation and learning insights
- **[Technical Evolution](technical/)** - Architecture evolution and implementation details

---

## 🆘 Getting Help

### **Common Issues**
- **Build errors**: Check PlatformIO installation and dependencies
- **Test failures**: Verify QEMU ARM emulation setup
- **Compilation issues**: Review C program syntax and Arduino API usage

### **Resources**
- **Documentation Hub**: [docs/README.md](README.md) - Complete navigation
- **GitHub Issues**: Report bugs and request features
- **Architecture Questions**: Review [Technical Documentation](technical/)

### **Development Support**
- **Phase Implementation**: [Development Documentation](development/)
- **Testing Framework**: [Testing History](development/testing/TESTING_HISTORY.md)
- **Debugging**: [Deep Dives](development/deep-dives/) for detailed analysis

---

## 🎓 Learning Path

### **Beginner (1-2 hours)**
1. Build and run existing examples
2. Write simple C programs (LED blink, printf)
3. Understand Arduino API compatibility

### **Intermediate (3-5 hours)**  
1. Review architecture documentation
2. Explore memory protection mechanisms
3. Understand bytecode compilation process

### **Advanced (5+ hours)**
1. Study component architecture and integration
2. Contribute to Phase 4 hardware transition
3. Extend instruction set or hardware support

---

*Ready to dive deeper? Start with the [Architecture Documentation](architecture/) or jump into [API Reference](API_REFERENCE_COMPLETE.md) for hands-on development.*
