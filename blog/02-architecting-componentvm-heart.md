# Architecting ComponentVM: Crafting a 32-bit Stack-Based Heart

## Purpose:
To dive into the core architectural decisions of ComponentVM, demonstrating your understanding of low-level design, instruction sets, and VM execution principles. This post is highly technical, showcasing your foundational embedded systems knowledge.

## Prompts:
* Walk through the high-level architecture of ComponentVM: the ExecutionEngine, MemoryManager, and IOController. Explain their roles and interdependencies.
* Detail the design of your 32-bit stack-based instruction set (`vm_instruction_t`). Why did you opt for a custom instruction set, and what were the key considerations for its opcode, flags, and immediate fields?
* Describe the memory layout (Flash/RAM allocation, System Stack vs. VM Memory). What trade-offs or constraints did this impose, and how did you optimize for resource efficiency on a Cortex-M4?
* Discuss the "Arduino-compatible Hardware Abstraction Layer (HAL)." How does this bridge allow higher-level C bytecode to interact with bare-metal peripherals, and what challenges did you overcome in its implementation?
