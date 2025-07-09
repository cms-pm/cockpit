# Debug Plan: Compiler-VM Integration Gap Analysis

## Problem Statement
Runtime validator shows "Invalid instruction" errors for all 13 test programs, but opcode handler table appears complete and all referenced opcodes (0x08 CALL, 0x00 HALT, 0x01 PUSH, 0x50 LOAD_GLOBAL, 0x51 STORE_GLOBAL) have proper handler implementations.

## Key Observations

### ✅ Handler Table Verification
- **Complete opcode coverage**: All opcodes 0x00-0x6F have either handlers or explicit nullptr
- **Critical opcodes implemented**: CALL(0x08), HALT(0x00), PUSH(0x01), LOAD_GLOBAL(0x50), STORE_GLOBAL(0x51)
- **No nullptr gaps**: Memory/bitwise operations properly implemented

### 🔍 Failure Analysis Required

#### Hypothesis 1: Function Call Mechanics
```
Program pattern: CALL(0x08, addr=2) -> HALT(0x00) -> setup() implementation
```
**Potential issues:**
- CALL pushes return address but jumps to invalid location
- RET instruction returns to wrong address 
- Stack corruption during function call

#### Hypothesis 2: Global Variable Access
```
Instructions: LOAD_GLOBAL(0x50, index=9), STORE_GLOBAL(0x51, index=9)
```
**Potential issues:**
- Global variable index 9 exceeds MemoryManager bounds
- Memory manager not properly initialized
- Invalid memory access triggering error

#### Hypothesis 3: Instruction Decoding
```
32-bit format: 0x08000002 -> opcode=0x08, flags=0x00, immediate=0x0002
```
**Potential issues:**
- Endianness mismatch between compiler and VM
- Instruction format interpretation errors
- Immediate value range validation failures

## Debug Execution Plan

### Phase 1: Isolate the Failure Point (15 minutes)
1. **Create minimal test program** without printf/complex operations
2. **Single-step execution** with detailed logging at each instruction
3. **Identify exact instruction** where execution fails

### Phase 2: Component Validation (15 minutes)  
1. **Test function call mechanics** in isolation (CALL/RET only)
2. **Test memory operations** in isolation (LOAD/STORE_GLOBAL only)
3. **Validate stack state** after each operation

### Phase 3: Root Cause Resolution (30 minutes)
1. **Fix identified issue** (likely stack corruption or bounds checking)
2. **Validate fix** with progressively complex test cases
3. **Confirm runtime validator** passes on basic programs

## Specific Debug Actions

### Action 1: Create Minimal Test Program
```c
// Simplest possible test - no printf, no complex operations
int global_var;

void setup() {
    global_var = 42;
}
```

### Action 2: Enhanced Logging in Runtime Validator
- Add logging after each instruction execution
- Log stack state, PC, and error conditions
- Identify the specific instruction causing failure

### Action 3: Targeted Component Tests
```c
// Test 1: Pure function call
void setup() { /* empty */ }

// Test 2: Pure memory operation  
int x; void setup() { x = 1; }

// Test 3: Combined operations
int x; void setup() { x = 1; x = x + 1; }
```

## Expected Outcomes

### Most Likely Root Causes
1. **Stack overflow during function calls** - CALL/RET corrupting stack
2. **Global variable bounds checking** - Accessing invalid memory indices  
3. **Return address corruption** - Function returns jumping to invalid PC

### Success Criteria
- Runtime validator executes simple programs successfully
- Complex programs (with printf) execute correctly
- All 13 test programs pass validation

## Timeline
- **Phase 1**: 15 minutes to isolate failure point
- **Phase 2**: 15 minutes for component validation  
- **Phase 3**: 30 minutes for fix and validation
- **Total**: 1 hour to resolve integration gap

This systematic approach should quickly identify and resolve the integration issue, enabling confident hardware transition.