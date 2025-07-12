# ComponentVM Naming Refactor Plan

**Goal:** Establish consistent `vm_*` naming hierarchy for all VM components
**Strategy:** Surgical rename with comprehensive testing and git safety

## Current State Analysis

### Existing Directory Structure
```
lib/
├── arduino_hal/               # Keep as-is (hardware abstraction)
├── component_vm/              # → vm_cockpit/
├── component_vm_bridge/       # → vm_bridge/
└── semihosting/              # Keep as-is (utility)

src/
├── main.c                    # Update includes
├── test_vm_hardware_integration.c  # Update includes
└── qemu_tests/               # Keep as-is
```

### File Dependencies Analysis
```bash
# Files that include component_vm headers:
grep -r "component_vm" --include="*.c" --include="*.cpp" --include="*.h"
```

**Primary Include Dependencies:**
- `src/test_vm_hardware_integration.c` includes `component_vm_bridge.h`
- `lib/component_vm_bridge/component_vm_bridge.cpp` includes ComponentVM headers
- `platformio.ini` references ComponentVM library

## Rename Strategy

### Phase 1: Preparation and Safety (30 minutes)

**1.1 Create Feature Branch**
```bash
git checkout -b feature/vm-naming-refactor
git status  # Verify clean working directory
```

**1.2 Comprehensive Backup**
```bash
# Create backup of current state
cp -r lib/ lib_backup_$(date +%Y%m%d_%H%M%S)
cp -r src/ src_backup_$(date +%Y%m%d_%H%M%S)
```

**1.3 Pre-Refactor Testing**
```bash
# Verify current build works
~/.platformio/penv/bin/pio run --environment weact_g431cb_hardware
# Record current build output for comparison
```

### Phase 2: Directory and File Renames (45 minutes)

**2.1 Rename Directories**
```bash
# Rename component_vm to vm_cockpit
git mv lib/component_vm lib/vm_cockpit

# Rename component_vm_bridge to vm_bridge  
git mv lib/component_vm_bridge lib/vm_bridge
```

**2.2 Update Include Paths in Headers**
```bash
# vm_bridge.h - update include paths
# Before: #include "../ComponentVM/include/component_vm.h"
# After:  #include "../vm_cockpit/include/component_vm.h"
```

**2.3 Update Include Statements in Source Files**
```bash
# src/test_vm_hardware_integration.c
# Before: #include "../lib/component_vm_bridge/component_vm_bridge.h"
# After:  #include "../lib/vm_bridge/vm_bridge.h"
```

### Phase 3: PlatformIO Configuration Updates (15 minutes)

**3.1 Update platformio.ini**
```ini
# Update library dependencies
lib_deps = 
    vm_cockpit
    # ... other dependencies

# Update any component_vm references in build flags
```

**3.2 Update Library Configuration Files**
```bash
# Check for any library.json or library.properties files
find . -name "library.*" -exec grep -l "component_vm" {} \;
```

### Phase 4: Documentation Updates (30 minutes)

**4.1 Update README and CLAUDE.md**
```bash
# Update all references to component_vm → vm_cockpit
# Update architecture diagrams and component descriptions
```

**4.2 Update Documentation References**
```bash
# Find all .md files with component_vm references
find docs/ -name "*.md" -exec grep -l "component_vm" {} \;
# Update systematically
```

### Phase 5: Testing and Validation (45 minutes)

**5.1 Build Testing**
```bash
# Clean build to ensure no cached references
~/.platformio/penv/bin/pio run --target clean --environment weact_g431cb_hardware

# Fresh build with new naming
~/.platformio/penv/bin/pio run --environment weact_g431cb_hardware

# Compare output with pre-refactor build
```

**5.2 Functional Testing**
```bash
# Upload and test LED blink functionality
~/.platformio/penv/bin/pio run --target upload --environment weact_g431cb_hardware

# Verify LED blinks at 500ms intervals (same as before)
```

**5.3 Link Testing**
```bash
# Verify all symbols resolve correctly
# Check for any undefined references
# Validate memory usage hasn't changed significantly
```

### Phase 6: Git Commit Strategy (15 minutes)

**6.1 Staged Commit Approach**
```bash
# Commit directory renames first
git add lib/vm_cockpit lib/vm_bridge
git rm -r lib/component_vm lib/component_vm_bridge
git commit -m "refactor: rename component directories to vm_* hierarchy

- component_vm → vm_cockpit (main VM engine)
- component_vm_bridge → vm_bridge (C++ to C interface)

Maintains consistent vm_* naming for alphabetical grouping"

# Commit include path updates
git add src/ 
git commit -m "refactor: update include paths for vm_* naming convention"

# Commit configuration updates
git add platformio.ini docs/
git commit -m "refactor: update build config and docs for vm_* naming"
```

**6.2 Testing Validation Commit**
```bash
git add .
git commit -m "test: validate vm_* refactor with hardware build and upload

- Build successful with new naming
- LED blink functionality preserved
- Memory usage unchanged
- All symbols resolve correctly"
```

## Risk Assessment and Mitigation

### High Risk Areas
**1. Include Path Dependencies**
- **Risk:** Broken includes after path changes
- **Mitigation:** Systematic grep/replace, compile testing at each step

**2. PlatformIO Library Resolution**
- **Risk:** PlatformIO can't find renamed libraries
- **Mitigation:** Clean build, verify library.json updates

**3. Hidden References**
- **Risk:** References in build files, scripts, or documentation
- **Mitigation:** Comprehensive grep search before starting

### Medium Risk Areas
**1. Case Sensitivity**
- **Risk:** Different behavior on case-sensitive filesystems
- **Mitigation:** Use git mv for all renames

**2. Cached Build Artifacts**
- **Risk:** Old cached references interfere
- **Mitigation:** Clean build after each major change

### Rollback Strategy
```bash
# If anything goes wrong:
git checkout main                    # Return to known good state
git branch -D feature/vm-naming-refactor  # Delete problematic branch
# Restore from backup if needed
```

## Success Criteria

### Technical Validation
- [ ] Clean build with no errors or warnings
- [ ] Successful upload to STM32G431CB hardware
- [ ] LED blinks at same 500ms intervals as before refactor
- [ ] Memory usage within ±100 bytes of original
- [ ] All git operations clean (no untracked files)

### Functional Validation
- [ ] ComponentVM execution unchanged
- [ ] Arduino HAL calls work identically
- [ ] SysTick timing preserved
- [ ] Error handling behavior identical

### Documentation Validation
- [ ] All references updated consistently
- [ ] Build instructions remain accurate
- [ ] Architecture diagrams reflect new naming
- [ ] No broken internal links

## File-by-File Checklist

### Files Requiring Include Path Updates
- [ ] `src/test_vm_hardware_integration.c`
- [ ] `lib/vm_bridge/vm_bridge.cpp` (renamed from component_vm_bridge.cpp)
- [ ] `lib/vm_bridge/vm_bridge.h` (renamed from component_vm_bridge.h)
- [ ] Any test files in `src/qemu_tests/`

### Configuration Files Requiring Updates
- [ ] `platformio.ini`
- [ ] Any `library.json` or `library.properties` files
- [ ] CMake files (if any)

### Documentation Files Requiring Updates
- [ ] `README.md`
- [ ] `CLAUDE.md`
- [ ] All files in `docs/` directory
- [ ] This refactor plan document

## Estimated Timeline
**Total Time:** 3 hours
- **Preparation:** 30 minutes
- **Execution:** 90 minutes  
- **Testing:** 45 minutes
- **Git Operations:** 15 minutes

## Post-Refactor Benefits
1. **Consistent Naming:** All VM components use `vm_*` prefix
2. **Alphabetical Grouping:** Related components group together in file listings
3. **Clear Ownership:** Immediate identification of VM vs hardware components
4. **Scalable Architecture:** Pattern established for future vm_* components
5. **Professional Organization:** Shows architectural discipline and planning

---

**Before Proceeding:** 
1. Verify current build works perfectly
2. Ensure git working directory is clean
3. Confirm no uncommitted changes exist
4. Review this plan for any missed dependencies

**Ready for execution approval?**