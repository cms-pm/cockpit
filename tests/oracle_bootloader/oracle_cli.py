#!/usr/bin/env python3
"""
Oracle Bootloader Testing CLI

Command-line interface for ComponentVM bootloader reliability testing.
Provides error injection, scenario composition, and recovery validation.

Usage:
    ./oracle_cli.py --scenario normal --device /dev/ttyUSB0 --verbose
    ./oracle_cli.py --sequence timeout_recovery_chain --device /dev/ttyUSB0 --json-output results.json
"""

import sys
import os
import argparse
import logging
import json
import time
from pathlib import Path

# Add lib directory to Python path
lib_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'lib')
sys.path.insert(0, lib_path)

from scenario_runner import OracleScenarioRunner, ScenarioResult, SequenceResult
from protocol_client import ProtocolClient

def setup_logging(verbose: bool = False, log_file: str = None):
    """Setup logging configuration."""
    level = logging.DEBUG if verbose else logging.INFO
    format_string = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    
    handlers = [logging.StreamHandler()]
    if log_file:
        handlers.append(logging.FileHandler(log_file))
    
    logging.basicConfig(
        level=level,
        format=format_string,
        handlers=handlers
    )

def format_scenario_result(result: ScenarioResult, verbose: bool = False) -> dict:
    """Format scenario result for JSON output."""
    formatted = {
        'success': result.success,
        'scenario_name': result.scenario_name,
        'message': result.message,
        'execution_time': round(result.execution_time, 3)
    }
    
    if verbose and result.data:
        formatted['data'] = result.data
    
    if result.error_details:
        formatted['error_details'] = result.error_details
    
    return formatted

def format_sequence_result(result: SequenceResult, verbose: bool = False) -> dict:
    """Format sequence result for JSON output."""
    formatted = {
        'success': result.success,
        'sequence_name': result.sequence_name,
        'message': result.message,
        'total_execution_time': round(result.total_execution_time, 3),
        'scenario_count': len(result.scenario_results),
        'scenarios': [format_scenario_result(sr, verbose) for sr in result.scenario_results]
    }
    
    if result.failed_at_scenario:
        formatted['failed_at_scenario'] = result.failed_at_scenario
    
    return formatted

def main():
    parser = argparse.ArgumentParser(
        description='Oracle Bootloader Testing CLI',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  # Flash programming (Phase 4.7)
  %(prog)s --flash bytecode.bin --device /dev/ttyUSB1
  %(prog)s --verify-only input.bin --device /dev/ttyUSB1 
  %(prog)s --readback output.bin --device /dev/ttyUSB1
  
  # Run single scenario
  %(prog)s --scenario normal --device /dev/ttyUSB0 --verbose
  
  # Run scenario sequence
  %(prog)s --sequence timeout_recovery_chain --device /dev/ttyUSB0
  
  # JSON output for automation
  %(prog)s --scenario normal --device /dev/ttyUSB0 --json-output results.json --batch-mode
  
  # List available tests
  %(prog)s --list-scenarios
        '''
    )
    
    # Test selection arguments
    test_group = parser.add_mutually_exclusive_group(required=False)
    test_group.add_argument('--scenario', 
                           help='Run single scenario by name')
    test_group.add_argument('--sequence', 
                           help='Run scenario sequence by name')
    test_group.add_argument('--list-scenarios', action='store_true',
                           help='List available scenarios and sequences')
    
    # Phase 4.7: Flash programming commands
    test_group.add_argument('--flash', metavar='BYTECODE_FILE',
                           help='Upload, program, and verify bytecode file to flash')
    test_group.add_argument('--verify-only', metavar='INPUT_FILE',
                           help='Compare live flash content against provided file (no programming)')
    test_group.add_argument('--readback', metavar='OUTPUT_FILE', 
                           help='Download flash content to file')
    
    # Hardware configuration
    parser.add_argument('--device', default='/dev/ttyUSB0',
                        help='Serial device path (default: /dev/ttyUSB0)')
    
    # Output options
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Enable verbose logging')
    parser.add_argument('--json-output', 
                        help='Write JSON results to specified file')
    parser.add_argument('--batch-mode', action='store_true',
                        help='Minimize interactive output for automation')
    
    # Configuration
    parser.add_argument('--scenarios-config',
                        default=os.path.join(os.path.dirname(__file__), 'scenarios', 'basic_scenarios.yaml'),
                        help='Path to scenarios configuration file')
    parser.add_argument('--log-file',
                        help='Write logs to specified file')
    
    args = parser.parse_args()
    
    # Setup logging
    setup_logging(args.verbose, args.log_file)
    logger = logging.getLogger('oracle_cli')
    
    if not args.batch_mode:
        print("=== Oracle Bootloader Testing CLI ===")
        print(f"Device: {args.device}")
        print(f"Config: {args.scenarios_config}")
        print()
    
    # Initialize Oracle runner
    runner = OracleScenarioRunner(args.device)
    
    # Load scenario configuration
    if not runner.load_scenarios(args.scenarios_config):
        logger.error("Failed to load scenario configuration")
        return 1
    
    # Handle list scenarios request
    if args.list_scenarios:
        print("Available Scenarios:")
        for name, config in runner.scenarios_config.items():
            desc = config.get('description', 'No description')
            print(f"  {name}: {desc}")
        
        print("\nAvailable Sequences:")
        for name, config in runner.sequences_config.items():
            desc = config.get('description', 'No description')
            print(f"  {name}: {desc}")
        
        return 0
    
    # Require test selection if not listing
    if not args.scenario and not args.sequence and not args.flash and not args.verify_only and not args.readback:
        parser.error("Must specify one of: --scenario, --sequence, --flash, --verify-only, --readback, or --list-scenarios")
    
    # Execute test
    start_time = time.time()
    success = False
    result_data = None
    
    try:
        if args.scenario:
            logger.info(f"Running scenario: {args.scenario}")
            if not runner.setup_protocol_connection():
                logger.error("Failed to setup protocol connection")
                return 1
            
            try:
                scenario_result = runner.execute_single_scenario(args.scenario)
                success = scenario_result.success
                result_data = format_scenario_result(scenario_result, args.verbose)
                
                if not args.batch_mode:
                    if success:
                        print(f"✓ Scenario '{args.scenario}' PASSED: {scenario_result.message}")
                    else:
                        print(f"✗ Scenario '{args.scenario}' FAILED: {scenario_result.message}")
                    print(f"Execution time: {scenario_result.execution_time:.3f}s")
                
            finally:
                runner.cleanup_connection()
        
        elif args.sequence:
            logger.info(f"Running sequence: {args.sequence}")
            if not runner.setup_protocol_connection():
                logger.error("Failed to setup protocol connection")
                return 1
            
            try:
                sequence_result = runner.execute_scenario_sequence(args.sequence)
                success = sequence_result.success
                result_data = format_sequence_result(sequence_result, args.verbose)
                
                if not args.batch_mode:
                    if success:
                        print(f"✓ Sequence '{args.sequence}' PASSED: {sequence_result.message}")
                    else:
                        print(f"✗ Sequence '{args.sequence}' FAILED: {sequence_result.message}")
                    print(f"Total execution time: {sequence_result.total_execution_time:.3f}s")
                    print(f"Scenarios executed: {len(sequence_result.scenario_results)}")
                    
                    # Show individual scenario results
                    for sr in sequence_result.scenario_results:
                        status = "PASS" if sr.success else "FAIL"
                        print(f"  - {sr.scenario_name}: {status}")
                
            finally:
                runner.cleanup_connection()
        
        # Phase 4.7: Flash programming commands
        elif args.flash:
            logger.info(f"Flashing bytecode file: {args.flash}")
            if not Path(args.flash).exists():
                logger.error(f"File not found: {args.flash}")
                return 1
            
            # Read bytecode file
            try:
                with open(args.flash, 'rb') as f:
                    bytecode_data = f.read()
            except Exception as e:
                logger.error(f"Failed to read file {args.flash}: {e}")
                return 1
            
            # Execute complete flash protocol
            client = ProtocolClient(args.device)
            if not client.connect():
                logger.error("Failed to connect to bootloader")
                return 1
            
            try:
                result = client.execute_complete_protocol(bytecode_data)
                success = result.success
                
                if not args.batch_mode:
                    if success:
                        print(f"✓ Flash programming SUCCESSFUL: {result.message}")
                        if result.data:
                            print(f"Bytes programmed: {result.data.get('test_data_size', 'unknown')}")
                    else:
                        print(f"✗ Flash programming FAILED: {result.message}")
                
                result_data = {
                    'success': success,
                    'message': result.message,
                    'file': args.flash,
                    'bytes_programmed': len(bytecode_data) if success else 0
                }
                
            finally:
                client.disconnect()
                
        elif args.verify_only:
            logger.info(f"Verifying flash against file: {args.verify_only}")
            if not Path(args.verify_only).exists():
                logger.error(f"File not found: {args.verify_only}")
                return 1
            
            if not args.batch_mode:
                print("TODO Phase 4.8: --verify-only not yet implemented")
                print("This command will compare live flash content against the provided file")
            
            result_data = {'success': False, 'message': 'Not implemented in Phase 4.7'}
            
        elif args.readback:
            logger.info(f"Reading back flash content to: {args.readback}")
            
            if not args.batch_mode:
                print("TODO Phase 4.8: --readback not yet implemented") 
                print("This command will download flash content to the specified file")
            
            result_data = {'success': False, 'message': 'Not implemented in Phase 4.7'}
    
    except KeyboardInterrupt:
        logger.info("Test interrupted by user")
        if not args.batch_mode:
            print("\nTest interrupted by user")
        return 1
        
    except Exception as e:
        logger.error(f"Unexpected error: {e}")
        if not args.batch_mode:
            print(f"Error: {e}")
        return 1
    
    # Write JSON output if requested
    if args.json_output and result_data:
        try:
            # Determine test type and name
            if args.scenario:
                test_type, test_name = 'scenario', args.scenario
            elif args.sequence:
                test_type, test_name = 'sequence', args.sequence
            elif args.flash:
                test_type, test_name = 'flash', args.flash
            elif args.verify_only:
                test_type, test_name = 'verify_only', args.verify_only
            elif args.readback:
                test_type, test_name = 'readback', args.readback
            else:
                test_type, test_name = 'unknown', 'unknown'
            
            output_data = {
                'oracle_version': '4.7.2A',
                'test_type': test_type,
                'test_name': test_name,
                'device': args.device,
                'timestamp': time.time(),
                'total_time': time.time() - start_time,
                'result': result_data
            }
            
            with open(args.json_output, 'w') as f:
                json.dump(output_data, f, indent=2)
            
            logger.info(f"Results written to {args.json_output}")
            
        except Exception as e:
            logger.error(f"Failed to write JSON output: {e}")
    
    if not args.batch_mode:
        total_time = time.time() - start_time
        print(f"\nTotal execution time: {total_time:.3f}s")
        print("Oracle testing complete.")
    
    return 0 if success else 1

if __name__ == '__main__':
    sys.exit(main())