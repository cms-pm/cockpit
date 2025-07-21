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
    if not args.scenario and not args.sequence:
        parser.error("Must specify --scenario, --sequence, or --list-scenarios")
    
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
            output_data = {
                'oracle_version': '4.5.2D',
                'test_type': 'scenario' if args.scenario else 'sequence',
                'test_name': args.scenario or args.sequence,
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