# Oracle Python-Based Bootloader Client

Python-based bootloader client for automated bootloader protocol testing with the Golden Triangle Test Framework and manual CLI operations. CockpitVM bootloader reliability testing with error injection and recovery validation.

## Features

- **Flash Safety Net**: All scenarios protected with automatic flash backup/restore
- **Error Injection**: Timeout scenarios, CRC corruption, frame parsing errors
- **Scenario Composition**: Compound test sequences (normal → error → recovery → normal)
- **Workspace Integration**: YAML-configured testing within workspace test suite
- **Hardware Reset**: pyOCD-based reset for clean state management

## Quick Start

```bash
# Setup virtual environment
python3 -m venv oracle_venv
source oracle_venv/bin/activate
pip install -r requirements.txt

# List available tests
./oracle_cli.py --list-scenarios

# Run single scenario
./oracle_cli.py --scenario normal --device /dev/ttyUSB0 --verbose

# Run compound sequence
./oracle_cli.py --sequence timeout_recovery_chain --device /dev/ttyUSB0

# JSON output for automation
./oracle_cli.py --scenario normal --device /dev/ttyUSB0 --json-output results.json --batch-mode
```

## Available Scenarios

### Single Scenarios
- **normal**: Baseline bootloader protocol validation (always should pass)
- **timeout_session**: Test 30-second session timeout recovery
- **timeout_handshake**: Test 2-second handshake timeout recovery  
- **crc_frame_corruption**: Test frame CRC16 corruption and recovery
- **partial_frame_timeout**: Test incomplete frame timeout recovery

### Compound Sequences
- **timeout_recovery_chain**: normal → timeout_session → normal
- **handshake_timeout_recovery_chain**: normal → timeout_handshake → normal
- **crc_recovery_chain**: normal → crc_frame_corruption → normal
- **comprehensive_error_chain**: Multiple error types with full recovery validation

## Workspace Integration

Add Oracle testing to test_catalog.yaml:

```yaml
flash_programming_protocol_oracle:
  platform: stm32g4
  source: test_flash_programming_protocol.c
  oracle_scenarios:
    - normal
    - timeout_session
    - crc_frame_corruption
  oracle_sequences:
    - timeout_recovery_chain
    - comprehensive_error_chain
```

## Architecture

- **pyOCD Manager**: Hardware reset and flash backup via SWD
- **Protocol Client**: Complete bootloader protocol over serial 
- **Error Injector**: Level 1 error injection (timeout, CRC corruption)
- **Scenario Runner**: Single and compound scenario execution
- **Flash Safety**: Automatic backup/restore for clean state recovery

## Reliability Features

- **30-second bootloader window**: Generous timing margin for complex scenarios
- **Flash backup before every scenario**: Complete safety net prevents data loss
- **Hardware reset recovery**: pyOCD reset for unresponsive bootloader recovery
- **Validation levels**: Basic connectivity → handshake → mini protocol → complete protocol
- **Compound testing**: Chains validate recovery between error conditions

## Success Metrics

- **Normal scenario**: 100% pass rate (validates Oracle tooling)
- **Error scenarios**: >90% successful recovery (validates bootloader reliability)
- **Sequence scenarios**: Complete recovery chain validation
- **Flash integrity**: No data corruption after any scenario execution