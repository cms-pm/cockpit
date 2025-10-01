# Oracle/Golden Triangle Team Implementation Guide - Enhanced Protocol V2.0

**Document Version**: 1.0  
**Target Platform**: Python CLI + Hardware-in-Loop Testing  
**Date**: 2025-09-07  
**Status**: Implementation Ready

## Overview

This guide provides specific implementation details for the Oracle CLI and Golden Triangle testing framework to integrate Enhanced Protocol V2.0 features while maintaining compatibility with existing test infrastructure.

## Shared Protocol Library Integration

### Python Bindings Architecture
```python
# bootloader_protocol/__init__.py - Python wrapper for shared C++ library
import bootloader_protocol_bindings as _bindings
from typing import List, Optional, Dict, Any
from dataclasses import dataclass
from enum import Enum

@dataclass
class DeviceInfo:
    device_model: str
    bootloader_version: str
    flash_total_size: int
    flash_page_size: int
    bootloader_region_end: int
    hypervisor_region_start: int
    hypervisor_region_end: int
    bytecode_region_start: int
    bytecode_region_end: int
    test_page_address: int
    unique_device_id: bytes
    hardware_revision: str

@dataclass
class FlashReadResult:
    flash_data: bytes
    actual_length: int
    data_crc32: int
    read_address: int
    chunk_sequence: int
    has_more_chunks: bool

@dataclass 
class VerificationReport:
    hash_verified: bool
    crc_verified: bool
    readback_verified: bool
    first_difference: Optional[bytes]
    difference_offset: int
    verification_status: str

class ProtocolClient:
    """Enhanced Protocol Client with V2.0 features"""
    
    def __init__(self, device_path: str, timeout_ms: int = 2000):
        self._client = _bindings.create_protocol_client(device_path, timeout_ms)
        self._device_path = device_path
        self._cached_device_info: Optional[DeviceInfo] = None
    
    # NEW V2.0 OPERATIONS
    def query_device_info(self, include_memory_layout: bool = True, 
                         include_device_id: bool = True) -> DeviceInfo:
        """Query comprehensive device information"""
        result = self._client.query_device_information(include_memory_layout, include_device_id)
        if not result.success:
            raise ProtocolError(result.error_message)
        
        self._cached_device_info = DeviceInfo(**result.device_info)
        return self._cached_device_info
    
    def read_flash_region(self, address: int, length: int, 
                         progress_callback=None) -> bytes:
        """Read flash memory in chunks with optional progress callback"""
        chunks = []
        bytes_read = 0
        chunk_sequence = 0
        
        while bytes_read < length:
            chunk_size = min(256, length - bytes_read)
            chunk_address = address + bytes_read
            
            result = self._client.read_flash_chunk(chunk_address, chunk_size, chunk_sequence)
            if not result.success:
                raise ProtocolError(f"Flash read failed at 0x{chunk_address:08X}: {result.error_message}")
            
            chunks.append(result.flash_data)
            bytes_read += result.actual_length
            chunk_sequence += 1
            
            if progress_callback:
                progress = int((bytes_read * 100) / length)
                progress_callback(progress)
            
            if not result.has_more_chunks:
                break
        
        return b''.join(chunks)
    
    def upload_with_verification(self, bytecode_data: bytes, 
                               enable_readback: bool = True) -> VerificationReport:
        """Upload bytecode with enhanced verification"""
        # Perform standard upload
        upload_result = self._client.upload_bytecode_enhanced(bytecode_data, enable_readback)
        if not upload_result.success:
            raise ProtocolError(upload_result.error_message)
        
        # Perform full verification if requested
        if enable_readback:
            return self.verify_bytecode(bytecode_data)
        else:
            return VerificationReport(
                hash_verified=upload_result.hash_verified,
                crc_verified=upload_result.crc_verified,
                readback_verified=False,
                first_difference=None,
                difference_offset=0,
                verification_status="Upload completed, readback verification skipped"
            )
    
    def verify_bytecode(self, original_bytecode: bytes) -> VerificationReport:
        """Perform comprehensive bytecode verification"""
        if not self._cached_device_info:
            self.query_device_info()
        
        # Read back the programmed flash region
        test_address = self._cached_device_info.test_page_address
        flash_contents = self.read_flash_region(test_address, len(original_bytecode))
        
        # Perform verification comparisons
        hash_match = self._verify_hash(original_bytecode, flash_contents)
        crc_match = self._verify_crc32(original_bytecode, flash_contents)
        
        # Byte-by-byte comparison
        readback_match = True
        first_diff = None
        diff_offset = 0
        
        for i, (orig, flash) in enumerate(zip(original_bytecode, flash_contents)):
            if orig != flash:
                readback_match = False
                first_diff = original_bytecode[i:i+8] + flash_contents[i:i+8]
                diff_offset = i
                break
        
        overall_status = "✅ VERIFIED" if (hash_match and crc_match and readback_match) else "❌ VERIFICATION FAILED"
        
        return VerificationReport(
            hash_verified=hash_match,
            crc_verified=crc_match,
            readback_verified=readback_match,
            first_difference=first_diff,
            difference_offset=diff_offset,
            verification_status=overall_status
        )
```

## Enhanced Oracle CLI Implementation

### Command Line Interface Updates
```python
# oracle_cli.py - Enhanced CLI with V2.0 commands
import click
import bootloader_protocol as bp
from rich.console import Console
from rich.table import Table
from rich.progress import Progress
import json

console = Console()

@click.group()
@click.option('--device', required=True, help='Serial device path')
@click.option('--verbose', is_flag=True, help='Enable verbose logging')
@click.pass_context
def cli(ctx, device, verbose):
    """Enhanced Oracle CLI with Protocol V2.0 support"""
    ctx.ensure_object(dict)
    ctx.obj['device'] = device
    ctx.obj['verbose'] = verbose
    ctx.obj['client'] = bp.ProtocolClient(device)

# NEW V2.0 COMMANDS

@cli.command()
@click.pass_context
def device_info(ctx):
    """Query comprehensive device information"""
    try:
        client = ctx.obj['client']
        info = client.query_device_info()
        
        # Display device information table
        table = Table(title="Device Information")
        table.add_column("Property", style="cyan")
        table.add_column("Value", style="magenta")
        
        table.add_row("Device Model", info.device_model)
        table.add_row("Bootloader Version", info.bootloader_version)
        table.add_row("Flash Total Size", f"{info.flash_total_size // 1024} KB")
        table.add_row("Flash Page Size", f"{info.flash_page_size} bytes")
        table.add_row("Test Page Address", f"0x{info.test_page_address:08X}")
        table.add_row("Unique Device ID", info.unique_device_id.hex(':').upper())
        table.add_row("Hardware Revision", info.hardware_revision)
        
        console.print(table)
        
        # Display memory layout
        memory_table = Table(title="Memory Layout")
        memory_table.add_column("Region", style="cyan")
        memory_table.add_column("Start Address", style="green")
        memory_table.add_column("End Address", style="red")
        memory_table.add_column("Size", style="yellow")
        
        memory_table.add_row("Bootloader", f"0x08000000", f"0x{info.bootloader_region_end:08X}", 
                            f"{(info.bootloader_region_end - 0x08000000 + 1) // 1024} KB")
        memory_table.add_row("Hypervisor", f"0x{info.hypervisor_region_start:08X}", 
                            f"0x{info.hypervisor_region_end:08X}",
                            f"{(info.hypervisor_region_end - info.hypervisor_region_start + 1) // 1024} KB")
        memory_table.add_row("Bytecode", f"0x{info.bytecode_region_start:08X}", 
                            f"0x{info.bytecode_region_end:08X}",
                            f"{(info.bytecode_region_end - info.bytecode_region_start + 1) // 1024} KB")
        
        console.print(memory_table)
        
    except bp.ProtocolError as e:
        console.print(f"[red]Device info query failed: {e}[/red]")

@cli.command()
@click.option('--address', type=lambda x: int(x, 0), required=True, help='Flash address to read (hex or decimal)')
@click.option('--length', type=int, required=True, help='Number of bytes to read')
@click.option('--output', type=click.File('wb'), help='Output file for flash data')
@click.option('--hex-dump', is_flag=True, help='Display hex dump')
@click.pass_context
def read_flash(ctx, address, length, output, hex_dump):
    """Read flash memory region"""
    try:
        client = ctx.obj['client']
        
        console.print(f"Reading {length} bytes from 0x{address:08X}...")
        
        with Progress() as progress:
            task = progress.add_task("Reading flash...", total=100)
            
            def progress_callback(percent):
                progress.update(task, completed=percent)
            
            flash_data = client.read_flash_region(address, length, progress_callback)
        
        console.print(f"[green]Successfully read {len(flash_data)} bytes[/green]")
        
        if output:
            output.write(flash_data)
            console.print(f"Flash data written to {output.name}")
        
        if hex_dump:
            _display_hex_dump(flash_data, address)
            
    except bp.ProtocolError as e:
        console.print(f"[red]Flash read failed: {e}[/red]")

@cli.command()
@click.argument('bytecode_file', type=click.File('rb'))
@click.option('--verify/--no-verify', default=True, help='Enable/disable readback verification')
@click.option('--json-output', type=click.File('w'), help='Output verification report as JSON')
@click.pass_context  
def upload_verified(ctx, bytecode_file, verify, json_output):
    """Upload bytecode with enhanced verification"""
    try:
        client = ctx.obj['client']
        bytecode_data = bytecode_file.read()
        
        console.print(f"Uploading {len(bytecode_data)} bytes with verification...")
        
        verification_report = client.upload_with_verification(bytecode_data, verify)
        
        # Display verification results
        _display_verification_report(verification_report)
        
        if json_output:
            report_dict = {
                'hash_verified': verification_report.hash_verified,
                'crc_verified': verification_report.crc_verified,
                'readback_verified': verification_report.readback_verified,
                'difference_offset': verification_report.difference_offset,
                'verification_status': verification_report.verification_status
            }
            json.dump(report_dict, json_output, indent=2)
            console.print(f"Verification report written to {json_output.name}")
            
    except bp.ProtocolError as e:
        console.print(f"[red]Upload failed: {e}[/red]")

def _display_verification_report(report: bp.VerificationReport):
    """Display formatted verification report"""
    table = Table(title="Verification Report")
    table.add_column("Check", style="cyan")
    table.add_column("Result", style="bold")
    table.add_column("Status")
    
    table.add_row("Hash (SHA-256)", "✅ PASS" if report.hash_verified else "❌ FAIL",
                  "Hash comparison successful" if report.hash_verified else "Hash mismatch detected")
    table.add_row("CRC32", "✅ PASS" if report.crc_verified else "❌ FAIL", 
                  "CRC32 checksum valid" if report.crc_verified else "CRC32 checksum invalid")
    table.add_row("Readback", "✅ PASS" if report.readback_verified else "❌ FAIL",
                  "Byte-by-byte match" if report.readback_verified else f"Difference at offset 0x{report.difference_offset:08X}")
    
    console.print(table)
    console.print(f"\n[bold]{report.verification_status}[/bold]")
    
    if report.first_difference:
        console.print(f"\nFirst difference at offset 0x{report.difference_offset:08X}:")
        expected = report.first_difference[:8].hex(' ')
        actual = report.first_difference[8:].hex(' ')
        console.print(f"Expected: {expected}")
        console.print(f"Actual:   {actual}")

def _display_hex_dump(data: bytes, base_address: int):
    """Display formatted hex dump"""
    console.print(f"\nHex dump (base address: 0x{base_address:08X}):")
    for i in range(0, len(data), 16):
        address = base_address + i
        chunk = data[i:i+16]
        hex_part = ' '.join(f'{b:02x}' for b in chunk)
        ascii_part = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)
        console.print(f"0x{address:08X}: {hex_part:<48} |{ascii_part}|")
```

## Golden Triangle Testing Framework Integration

### Enhanced Test Scenarios
```python
# tests/test_scenarios_v2.py - Enhanced test scenarios with V2.0 features
import pytest
import bootloader_protocol as bp
from golden_triangle_framework import HardwareTestFixture
import yaml

class TestEnhancedProtocolV2:
    """Test suite for Enhanced Bootloader Protocol V2.0"""
    
    @pytest.fixture
    def hardware_fixture(self):
        """Hardware-in-loop test fixture with real STM32G4"""
        return HardwareTestFixture(
            target_device="STM32G431CB",
            serial_port="/dev/ttyUSB0",
            swd_interface="stlink"
        )
    
    @pytest.fixture
    def protocol_client(self, hardware_fixture):
        """Protocol client connected to hardware"""
        return bp.ProtocolClient(hardware_fixture.serial_port)
    
    def test_device_info_query(self, protocol_client):
        """Test comprehensive device information query"""
        device_info = protocol_client.query_device_info()
        
        # Validate device information
        assert device_info.device_model == "STM32G431CB"
        assert device_info.bootloader_version.startswith("4.6.0-enhanced")
        assert device_info.flash_total_size == 131072  # 128KB
        assert device_info.flash_page_size == 2048     # 2KB pages
        assert device_info.test_page_address == 0x0801F800  # Page 63
        assert len(device_info.unique_device_id) == 12  # 96-bit UID
        
        # Validate memory regions
        assert device_info.bootloader_region_end == 0x08003FFF
        assert device_info.hypervisor_region_start == 0x08004000
        assert device_info.bytecode_region_end == 0x0801FFFF
    
    def test_flash_readback_operations(self, protocol_client):
        """Test chunked flash reading with various scenarios"""
        # Test small read (single chunk)
        small_data = protocol_client.read_flash_region(0x0801F800, 64)
        assert len(small_data) == 64
        
        # Test large read (multiple chunks)  
        large_data = protocol_client.read_flash_region(0x0801F800, 1024)
        assert len(large_data) == 1024
        
        # Test full page read
        page_data = protocol_client.read_flash_region(0x0801F800, 2048)
        assert len(page_data) == 2048
        
        # Test address bounds validation
        with pytest.raises(bp.ProtocolError, match="address.*invalid"):
            protocol_client.read_flash_region(0x08020000, 256)  # Beyond valid range
    
    @pytest.mark.parametrize("bytecode_file", [
        "test_vectors/bytecode_small.bin",      # < 256 bytes
        "test_vectors/bytecode_medium.bin",     # ~1KB  
        "test_vectors/bytecode_large.bin",      # ~2KB (full page)
    ])
    def test_enhanced_upload_verification(self, protocol_client, bytecode_file):
        """Test upload with full verification for various bytecode sizes"""
        with open(bytecode_file, 'rb') as f:
            bytecode_data = f.read()
        
        # Perform upload with verification
        verification_report = protocol_client.upload_with_verification(bytecode_data)
        
        # Validate verification results
        assert verification_report.hash_verified, "SHA-256 hash verification failed"
        assert verification_report.crc_verified, "CRC32 verification failed"  
        assert verification_report.readback_verified, "Readback verification failed"
        assert verification_report.first_difference is None, "Unexpected differences found"
        assert "VERIFIED" in verification_report.verification_status
    
    def test_protocol_branch_transitions(self, protocol_client):
        """Test two-branch protocol state machine"""
        # Test Branch A: Query-only session
        device_info = protocol_client.query_device_info()
        flash_data = protocol_client.read_flash_region(0x0801F800, 256)
        # Should be able to continue with more queries
        more_flash_data = protocol_client.read_flash_region(0x08000000, 256)
        
        # Test Branch B: Full programming session  
        bytecode = b'\x01\x02\x03\x04' * 64  # 256 bytes test data
        verification_report = protocol_client.upload_with_verification(bytecode)
        assert verification_report.readback_verified
    
    def test_cross_implementation_compatibility(self, hardware_fixture):
        """Test compatibility between Oracle CLI and Canopy implementations"""
        # Upload using Oracle CLI
        oracle_result = self._run_oracle_cli(
            ["upload-verified", "test_vectors/compatibility_test.bin", "--json-output", "/tmp/oracle_result.json"]
        )
        
        # Read back using Canopy protocol client
        canopy_client = bp.ProtocolClient(hardware_fixture.serial_port)
        canopy_data = canopy_client.read_flash_region(0x0801F800, 256)
        
        # Compare results
        with open("test_vectors/compatibility_test.bin", 'rb') as f:
            expected_data = f.read()
        
        assert canopy_data[:len(expected_data)] == expected_data, "Cross-implementation compatibility failed"

    def test_performance_benchmarks(self, protocol_client):
        """Test performance requirements for V2.0 operations"""
        import time
        
        # Device info query performance
        start_time = time.time()
        protocol_client.query_device_info()
        device_info_time = (time.time() - start_time) * 1000
        assert device_info_time < 50, f"Device info query took {device_info_time}ms (>50ms limit)"
        
        # Flash read performance (256 bytes)
        start_time = time.time()
        protocol_client.read_flash_region(0x0801F800, 256)
        flash_read_time = (time.time() - start_time) * 1000
        assert flash_read_time < 100, f"Flash read took {flash_read_time}ms (>100ms limit)"
        
        # Full upload + verification performance
        bytecode = b'\xAA' * 1024  # 1KB test data
        start_time = time.time()
        protocol_client.upload_with_verification(bytecode)
        upload_time = (time.time() - start_time) * 1000
        assert upload_time < 500, f"Enhanced upload took {upload_time}ms (>500ms limit)"
```

### Test Vector Management
```yaml
# test_vectors/enhanced_protocol_scenarios.yaml
scenarios:
  device_info_basic:
    description: "Basic device information query"
    expected_device_model: "STM32G431CB"
    expected_bootloader_version_prefix: "4.6.0-enhanced"
    expected_flash_size: 131072
    expected_page_size: 2048
    
  flash_read_bounds_testing:
    description: "Test flash read address bounds validation"
    valid_addresses:
      - { address: 0x08000000, length: 256, region: "bootloader" }
      - { address: 0x08004000, length: 256, region: "hypervisor" }  
      - { address: 0x0801F800, length: 256, region: "test_page" }
    invalid_addresses:
      - { address: 0x08020000, length: 256, expected_error: "FLASH_READ_ADDRESS_INVALID" }
      - { address: 0x07000000, length: 256, expected_error: "FLASH_READ_ADDRESS_INVALID" }
      
  verification_comprehensive:
    description: "Comprehensive verification testing"
    test_cases:
      - { file: "bytecode_valid.bin", expected_result: "all_verified" }
      - { file: "bytecode_corrupted.bin", expected_result: "verification_failed" }
      - { file: "bytecode_empty.bin", expected_result: "hash_mismatch" }
```

### Golden Triangle Hardware Integration
```python
# golden_triangle_framework.py - Enhanced hardware test framework
class HardwareTestFixture:
    """Hardware-in-loop test fixture with STM32G4 support"""
    
    def __init__(self, target_device: str, serial_port: str, swd_interface: str):
        self.target_device = target_device
        self.serial_port = serial_port
        self.swd_interface = swd_interface
        self._pyocd_session = None
        
    def reset_target_hardware(self):
        """Reset STM32G4 target via SWD"""
        import pyocd
        self._pyocd_session = pyocd.Session(self.swd_interface)
        self._pyocd_session.board.target.reset()
        
    def verify_flash_programming_hardware(self, address: int, expected_data: bytes) -> bool:
        """Hardware verification of flash programming via SWD"""
        if not self._pyocd_session:
            self.reset_target_hardware()
            
        actual_data = self._pyocd_session.board.target.read_memory_block8(address, len(expected_data))
        return bytes(actual_data) == expected_data
        
    def inject_flash_corruption(self, address: int, corrupt_bytes: bytes):
        """Inject flash corruption for negative testing"""
        if not self._pyocd_session:
            self.reset_target_hardware()
            
        self._pyocd_session.board.target.write_memory_block8(address, corrupt_bytes)
```

## Oracle Guest Bytecode Compilation Integration (Phase 4.14.4)

### Overview

The Oracle bootloader testing system now supports automatic compilation and flashing of ArduinoC guest programs with auto-execution header wrapping. This enables end-to-end testing of the complete guest execution pipeline: **ArduinoC Source → Bytecode Compilation → Auto-Execution Wrapping → Oracle Flash → Page 63 → ComponentVM Execution**.

### Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                  Oracle Test Scenario Configuration              │
│  (scenarios/basic_scenarios.yaml)                               │
│                                                                   │
│  flash_phase_4_9_4_bytecode:                                    │
│    compile_source: "test_registry/guest_programs/blinky.c"     │
│    wrap_for_auto_execution: true                                │
│    flash_target: "page_63"                                      │
└────────────────────┬────────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────────┐
│              Oracle Scenario Runner Integration                  │
│  (oracle_bootloader/lib/scenario_runner.py)                    │
│                                                                   │
│  _compile_and_wrap_bytecode()                                   │
│    ├─ Resolves source path relative to tests/                  │
│    ├─ Calls tools/bytecode_compiler.py --wrap                  │
│    └─ Returns wrapped bytecode as bytes                        │
└────────────────────┬────────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────────┐
│              Bytecode Compiler with Auto-Exec Wrapper            │
│  (tests/tools/bytecode_compiler.py)                            │
│                                                                   │
│  wrap_bytecode_for_auto_execution()                             │
│    ├─ Read raw bytecode (instruction_count, string_count, ...)│
│    ├─ Calculate CRC16 of bytecode data                         │
│    ├─ Create 16-byte auto-execution header:                    │
│    │   ┌──────────────────────────────────────────────────┐  │
│    │   │ uint32_t magic_signature   = 0x434F4E43 ("CONC") │  │
│    │   │ uint32_t program_size       = bytecode length    │  │
│    │   │ uint32_t instruction_count  = from bytecode      │  │
│    │   │ uint16_t string_count       = from bytecode      │  │
│    │   │ uint16_t crc16_checksum     = calculated CRC16   │  │
│    │   └──────────────────────────────────────────────────┘  │
│    └─ Write header + bytecode to *_wrapped.bin               │
└────────────────────┬────────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────────┐
│              Oracle Protocol Client Flash                        │
│  → Page 63 (0x0801F800)                                         │
└────────────────────┬────────────────────────────────────────────┘
                     │
                     ▼
┌─────────────────────────────────────────────────────────────────┐
│          ComponentVM Auto-Execution with Enhanced DIAG           │
│  (lib/vm_cockpit/src/vm_auto_execution.cpp)                    │
│                                                                   │
│  Forensic Page 63 Analysis:                                     │
│    ├─ PAGE63_ANALYSIS: Reading from address 0x0801f800         │
│    ├─ MAGIC_CHECK: Found=0x434f4e43 Expected=0x434f4e43 ✓     │
│    ├─ PAGE63_HEX: 434e4f43 7a010000 08000000 ...              │
│    └─ Guest bytecode execution with real-time telemetry       │
└─────────────────────────────────────────────────────────────────┘
```

### Scenario Configuration

Add guest bytecode scenarios to `scenarios/basic_scenarios.yaml`:

```yaml
scenarios:
  flash_phase_4_9_4_bytecode:
    type: "single"
    description: "Compile and flash ArduinoC guest bytecode for auto-execution"
    error_type: "none"
    protocol_flow:
      - handshake
      - prepare
      - data_transfer
      - verify
    # Custom bytecode compilation and wrapping
    compile_source: "test_registry/guest_programs/blinky_basic.c"
    wrap_for_auto_execution: true
    flash_target: "page_63"  # 0x0801F800
    expected_result: "success"
    validation_level: "complete_protocol"
    potentially_destructive: true  # Writes to flash Page 63
```

### Test Configuration

Reference the scenario in `test_catalog.yaml`:

```yaml
phase_4_9_4_auto_execution_complete:
  source: test_phase_4_9_4_auto_execution_complete.c
  description: "End-to-end auto-execution validation with Oracle integration"
  timeout: 120s
  semihosting: true
  oracle_scenarios:
    - "flash_phase_4_9_4_bytecode"  # Auto-compile and flash
  expected_patterns:
    - "Oracle Result: BYTECODE FLASHED SUCCESSFULLY ✓"
    - "✓ Guest program detected at Page 63"
    - "✓ Guest program execution initiated"
```

### Bytecode Compiler CLI

Standalone compilation with auto-execution wrapper:

```bash
# Compile and wrap for auto-execution
cd tests/tools
python3 bytecode_compiler.py ../test_registry/guest_programs/blinky_basic.c --wrap

# Output:
# ✅ Compilation successful!
#    Bytecode: ../test_registry/guest_programs/blinky_basic.bin
#    Instructions: 8
#    Strings: 2
#
# 📦 Wrapping bytecode for auto-execution...
# ✅ Wrapped bytecode created
#    Magic: 0x434F4E43, Size: 378, Instructions: 8, Strings: 2, CRC16: 0x5733
```

### Auto-Execution Header Format

```c
// lib/vm_cockpit/src/vm_auto_execution.h
typedef struct {
    uint32_t magic_signature;   // 0x434F4E43 ("CONC")
    uint32_t program_size;      // Size of bytecode in bytes
    uint32_t instruction_count; // Number of VM instructions
    uint16_t string_count;      // Number of string literals
    uint16_t crc16_checksum;    // CRC16 of bytecode data
} vm_auto_execution_header_t;  // 16 bytes total
```

### Enhanced Observability

ComponentVM now provides forensic Page 63 analysis via DIAG UART2:

```c
// Forensic telemetry output
[DEBUG] PAGE63_ANALYSIS: Reading from address 0x0801f800
[DEBUG] MAGIC_CHECK: Found=0x434f4e43 Expected=0x434f4e43
[INFO ] PAGE63_HEX: 434e4f43 7a010000 08000000 02000033 57080002

// Real-time execution telemetry
[DEBUG] EXEC[1]: PC=0 opcode=0x17 operand=0x00010013
[INFO ] GPIO_MODE: pin=13 mode=1 (PC=0)
[DEBUG] EXEC[2]: PC=1 opcode=0x10 operand=0x00010013
[INFO ] GPIO_WRITE: pin=13 value=1 (PC=1)
[DEBUG] EXEC[3]: PC=2 opcode=0x18 operand=0x000001f4
[INFO ] DELAY: 500 ms (PC=2)
```

### Test Execution Flow

```bash
# Run complete end-to-end test
cd tests
./tools/run_test phase_4_9_4_auto_execution_complete

# Automatic flow:
# 1. Workspace manager uploads host firmware to STM32G4
# 2. Oracle integration detects oracle_scenarios in test config
# 3. Scenario runner compiles blinky_basic.c with --wrap
# 4. Oracle protocol client flashes wrapped bytecode to Page 63
# 5. Host firmware auto-execution detects magic signature
# 6. ComponentVM executes guest bytecode with telemetry
# 7. Test validates GPIO operations and execution completion
```

### Troubleshooting

**Issue**: `Source file not found`
- **Cause**: Incorrect path in `compile_source`
- **Fix**: Path must be relative to `tests/` directory
- **Example**: `test_registry/guest_programs/blinky_basic.c` (not `../test_registry/...`)

**Issue**: `Magic signature not found in Page 63`
- **Cause**: Bytecode not wrapped with auto-execution header
- **Fix**: Ensure `wrap_for_auto_execution: true` in scenario config
- **Verify**: Check PAGE63_HEX starts with `434e4f43` ("CONC" in little-endian)

**Issue**: `CRC mismatch in Page 63 bytecode`
- **Cause**: Flash corruption during Oracle transfer
- **Fix**: Check Oracle protocol logs for transmission errors
- **Debug**: Use forensic hex dump to compare expected vs actual data

### Implementation Checklist

Oracle Integration:
- [x] Bytecode compilation integrated into scenario_runner.py
- [x] Auto-execution header wrapper in bytecode_compiler.py
- [x] Scenario configuration supports compile_source parameter
- [x] Test catalog references Oracle scenarios correctly

Enhanced Observability:
- [x] Forensic Page 63 analysis with hex dump
- [x] Magic signature validation telemetry
- [x] Real-time ComponentVM execution monitoring
- [x] Canonical vm_error.h error code integration

Testing:
- [ ] End-to-end test passes with automatic Oracle trigger
- [ ] Guest bytecode executes correctly on hardware
- [ ] GPIO operations validated via hardware monitoring
- [ ] Error conditions properly diagnosed via DIAG output

## Compliance and Testing Checklist

### Oracle CLI Integration
- [ ] All existing Oracle CLI commands maintain compatibility
- [ ] New V2.0 commands (device-info, read-flash, upload-verified) implemented
- [ ] JSON output format maintains consistency for automation
- [ ] Progress indication works for long-running operations
- [ ] Error handling provides actionable feedback

### Golden Triangle Testing  
- [ ] Hardware-in-loop tests validate real STM32G4 behavior
- [ ] Cross-implementation compatibility verified (Oracle ↔ Canopy)
- [ ] Performance benchmarks meet specification requirements  
- [ ] Test vectors cover all protocol edge cases
- [ ] Negative testing validates error handling

### Shared Library Integration
- [ ] Python bindings expose all C++ functionality
- [ ] Memory management prevents leaks in long-running tests
- [ ] Exception handling translates C++ errors to Python exceptions
- [ ] Progress callbacks work correctly for chunked operations

---

**Implementation Authority**: Enhanced bootloader protocol specification  
**Status**: Ready for Oracle/Golden Triangle Claude agent implementation