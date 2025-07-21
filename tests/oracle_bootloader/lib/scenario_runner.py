"""
Scenario Runner for Oracle Bootloader Testing

Orchestrates execution of single scenarios and compound scenario sequences.
Provides flash backup safety net and comprehensive validation.
"""

import time
import yaml
import logging
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
# pyOCD removed - workspace handles hardware management
from protocol_client import ProtocolClient, ProtocolResult
from error_injector import ErrorScenarioRunner

logger = logging.getLogger(__name__)

@dataclass
class ScenarioResult:
    """Result of single scenario execution."""
    success: bool
    scenario_name: str
    message: str
    execution_time: float
    data: Optional[Dict[str, Any]] = None
    error_details: Optional[str] = None

@dataclass 
class SequenceResult:
    """Result of scenario sequence execution."""
    success: bool
    sequence_name: str
    message: str
    total_execution_time: float
    scenario_results: List[ScenarioResult]
    failed_at_scenario: Optional[str] = None

class TestDataGenerator:
    """Generates test data patterns for Oracle scenarios."""
    
    @staticmethod
    def generate_test_data(size: int, pattern: str = "incremental") -> bytes:
        """
        Generate test data according to specified pattern.
        
        Args:
            size: Number of bytes to generate
            pattern: Pattern type ("incremental", "random_seed_42", etc.)
            
        Returns:
            Generated test data bytes
        """
        if pattern == "incremental":
            return bytes(i % 256 for i in range(size))
        elif pattern == "random_seed_42":
            import random
            random.seed(42)
            return bytes(random.randint(0, 255) for _ in range(size))
        elif pattern == "alternating":
            return bytes(0xAA if i % 2 == 0 else 0x55 for i in range(size))
        else:
            # Default to incremental
            return bytes(i % 256 for i in range(size))

class ScenarioValidator:
    """Validates scenario execution according to specified validation level."""
    
    def __init__(self, protocol_client: ProtocolClient):
        self.client = protocol_client
    
    def validate_scenario(self, validation_level: str) -> ProtocolResult:
        """
        Validate scenario execution according to specified level.
        
        Args:
            validation_level: Level of validation to perform
            
        Returns:
            ProtocolResult with validation outcome
        """
        logger.debug(f"Performing validation level: {validation_level}")
        
        if validation_level == "basic":
            return self._validate_basic()
        elif validation_level == "recovery_handshake":
            return self._validate_recovery_handshake()
        elif validation_level == "recovery_protocol":
            return self._validate_recovery_protocol()
        elif validation_level == "complete_protocol":
            return self._validate_complete_protocol()
        else:
            logger.warning(f"Unknown validation level: {validation_level}")
            return self._validate_basic()
    
    def _validate_basic(self) -> ProtocolResult:
        """Basic validation - test serial connectivity."""
        if self.client.serial_conn and self.client.serial_conn.is_open:
            return ProtocolResult(True, "Basic validation: Serial connection active")
        else:
            return ProtocolResult(False, "Basic validation: Serial connection failed")
    
    def _validate_recovery_handshake(self) -> ProtocolResult:
        """Recovery validation with handshake test."""
        try:
            result = self.client.execute_handshake()
            if result.success:
                return ProtocolResult(True, "Recovery validation: Handshake successful")
            else:
                return ProtocolResult(False, f"Recovery validation: Handshake failed - {result.message}")
        except Exception as e:
            return ProtocolResult(False, f"Recovery validation error: {e}")
    
    def _validate_recovery_protocol(self) -> ProtocolResult:
        """Recovery validation with mini protocol sequence."""
        try:
            # Test handshake + prepare sequence
            handshake_result = self.client.execute_handshake()
            if not handshake_result.success:
                return ProtocolResult(False, f"Recovery protocol: Handshake failed - {handshake_result.message}")
            
            prepare_result = self.client.execute_prepare_flash(128)
            if not prepare_result.success:
                return ProtocolResult(False, f"Recovery protocol: Prepare failed - {prepare_result.message}")
            
            return ProtocolResult(True, "Recovery validation: Mini protocol successful")
        except Exception as e:
            return ProtocolResult(False, f"Recovery protocol error: {e}")
    
    def _validate_complete_protocol(self) -> ProtocolResult:
        """Complete protocol validation."""
        try:
            test_data = TestDataGenerator.generate_test_data(128, "incremental")
            result = self.client.execute_complete_protocol(test_data)
            
            if result.success:
                return ProtocolResult(True, "Complete protocol validation successful")
            else:
                return ProtocolResult(False, f"Complete protocol validation failed: {result.message}")
        except Exception as e:
            return ProtocolResult(False, f"Complete protocol validation error: {e}")

class OracleScenarioRunner:
    """
    Main Oracle scenario runner with flash safety net and comprehensive validation.
    """
    
    def __init__(self, device_path: str):
        self.device_path = device_path
        # Remove pyOCD manager - workspace handles hardware reset
        self.protocol_client = ProtocolClient(device_path)
        self.error_runner = ErrorScenarioRunner(self.protocol_client)
        self.validator = ScenarioValidator(self.protocol_client)
        self.scenarios_config = {}
        self.sequences_config = {}
    
    def load_scenarios(self, config_file: str) -> bool:
        """
        Load scenario configuration from YAML file.
        
        Args:
            config_file: Path to scenario configuration file
            
        Returns:
            True if loaded successfully
        """
        try:
            with open(config_file, 'r') as f:
                config = yaml.safe_load(f)
            
            self.scenarios_config = config.get('scenarios', {})
            self.sequences_config = config.get('sequences', {})
            
            logger.info(f"Loaded {len(self.scenarios_config)} scenarios and {len(self.sequences_config)} sequences")
            return True
            
        except Exception as e:
            logger.error(f"Failed to load scenario config {config_file}: {e}")
            return False
    
    def setup_protocol_connection(self) -> bool:
        """
        Setup serial connection to bootloader.
        Assumes hardware reset was already performed by workspace test suite.
        
        Returns:
            True if connection successful
        """
        logger.info("Setting up Oracle protocol connection...")
        
        # Assume workspace test suite has already reset hardware
        logger.info("Assuming bootloader is running (reset handled by workspace)")
        
        # Wait for bootloader startup
        time.sleep(2.0)
        
        # Connect to bootloader via serial
        if not self.protocol_client.connect():
            logger.error("Failed to connect to bootloader via serial")
            return False
        
        logger.info("Oracle protocol connection established")
        return True
    
    def cleanup_connection(self):
        """Clean up protocol connections."""
        logger.info("Cleaning up Oracle connections...")
        self.protocol_client.disconnect()
    
    def execute_single_scenario(self, scenario_name: str) -> ScenarioResult:
        """
        Execute single Oracle scenario with flash safety net.
        
        Args:
            scenario_name: Name of scenario to execute
            
        Returns:
            ScenarioResult with execution outcome
        """
        start_time = time.time()
        logger.info(f"Executing scenario: {scenario_name}")
        
        # Get scenario configuration
        scenario_config = self.scenarios_config.get(scenario_name)
        if not scenario_config:
            return ScenarioResult(
                False, scenario_name, f"Scenario '{scenario_name}' not found",
                0.0, error_details="Missing scenario configuration"
            )
        
        try:
            # No hardware reset needed - workspace handles this
            logger.info(f"Running scenario '{scenario_name}' (hardware managed by workspace)")
            
            # Execute the specific scenario
            result = self._run_scenario_by_type(scenario_name, scenario_config)
            
            execution_time = time.time() - start_time
            
            if result.success:
                # Perform validation according to scenario configuration
                validation_level = scenario_config.get('validation_level', 'basic')
                validation_result = self.validator.validate_scenario(validation_level)
                
                if validation_result.success:
                    logger.info(f"Scenario '{scenario_name}' completed successfully")
                    return ScenarioResult(
                        True, scenario_name, f"Scenario successful: {result.message}",
                        execution_time, {"validation": validation_result.message}
                    )
                else:
                    logger.warning(f"Scenario '{scenario_name}' succeeded but validation failed")
                    return ScenarioResult(
                        False, scenario_name, f"Validation failed: {validation_result.message}",
                        execution_time, error_details="Post-scenario validation failed"
                    )
            else:
                logger.error(f"Scenario '{scenario_name}' failed: {result.message}")
                return ScenarioResult(
                    False, scenario_name, f"Scenario failed: {result.message}",
                    execution_time, error_details=result.error_code
                )
                
        except Exception as e:
            execution_time = time.time() - start_time
            logger.error(f"Exception in scenario '{scenario_name}': {e}")
            return ScenarioResult(
                False, scenario_name, f"Exception: {str(e)}",
                execution_time, error_details=str(e)
            )
        
        finally:
            # No flash restore needed - workspace handles hardware state
            pass
    
    def _run_scenario_by_type(self, scenario_name: str, scenario_config: Dict[str, Any]) -> ProtocolResult:
        """
        Run scenario based on its configuration type.
        
        Args:
            scenario_name: Name of scenario
            scenario_config: Scenario configuration dictionary
            
        Returns:
            ProtocolResult with scenario outcome
        """
        error_type = scenario_config.get('error_type', 'none')
        
        if error_type == 'none':
            # Normal protocol execution
            test_size = scenario_config.get('test_payload_size', 256)
            test_data = TestDataGenerator.generate_test_data(test_size, "incremental")
            return self.protocol_client.execute_complete_protocol(test_data)
        
        elif error_type == 'timeout':
            # Timeout error injection
            timeout_seconds = scenario_config.get('timeout_seconds', 35.0)
            injection_point = scenario_config.get('injection_point', 'after_handshake')
            
            if injection_point == 'after_handshake':
                return self.error_runner.run_error_scenario('timeout_session', delay_seconds=timeout_seconds)
            elif injection_point == 'before_handshake':
                return self.error_runner.run_error_scenario('timeout_handshake', delay_seconds=timeout_seconds)
            elif injection_point == 'partial_frame':
                return self.error_runner.run_error_scenario('partial_frame_timeout', delay_seconds=timeout_seconds)
        
        elif error_type == 'crc_corruption':
            # CRC corruption error injection
            return self.error_runner.run_error_scenario('crc_frame_corruption')
        
        else:
            return ProtocolResult(False, f"Unknown error type: {error_type}")
    
    def execute_scenario_sequence(self, sequence_name: str) -> SequenceResult:
        """
        Execute compound scenario sequence.
        
        Args:
            sequence_name: Name of scenario sequence to execute
            
        Returns:
            SequenceResult with sequence outcome
        """
        start_time = time.time()
        logger.info(f"Executing scenario sequence: {sequence_name}")
        
        # Get sequence configuration
        sequence_config = self.sequences_config.get(sequence_name)
        if not sequence_config:
            return SequenceResult(
                False, sequence_name, f"Sequence '{sequence_name}' not found",
                0.0, [], failed_at_scenario="configuration_missing"
            )
        
        sequence_steps = sequence_config.get('sequence', [])
        scenario_results = []
        
        for step in sequence_steps:
            if isinstance(step, str):
                # Simple scenario name
                step_scenario = step
                step_description = f"Execute {step}"
            elif isinstance(step, dict):
                # Step with description
                step_scenario = step.get('scenario')
                step_description = step.get('description', f"Execute {step_scenario}")
            else:
                logger.error(f"Invalid sequence step format: {step}")
                continue
            
            logger.info(f"Sequence step: {step_description}")
            
            # Execute scenario step
            step_result = self.execute_single_scenario(step_scenario)
            scenario_results.append(step_result)
            
            if not step_result.success:
                # Sequence failed at this step
                total_time = time.time() - start_time
                logger.error(f"Sequence '{sequence_name}' failed at scenario '{step_scenario}'")
                
                return SequenceResult(
                    False, sequence_name, 
                    f"Sequence failed at scenario '{step_scenario}': {step_result.message}",
                    total_time, scenario_results, failed_at_scenario=step_scenario
                )
        
        # All steps successful
        total_time = time.time() - start_time
        logger.info(f"Sequence '{sequence_name}' completed successfully")
        
        return SequenceResult(
            True, sequence_name, f"All {len(scenario_results)} scenarios successful",
            total_time, scenario_results
        )
    
    def run_oracle_test(self, test_name: str) -> bool:
        """
        Main entry point for running Oracle tests (single scenario or sequence).
        
        Args:
            test_name: Name of scenario or sequence to run
            
        Returns:
            True if test successful
        """
        logger.info(f"Starting Oracle test: {test_name}")
        
        # Setup protocol connection (hardware reset handled by workspace)
        if not self.setup_protocol_connection():
            logger.error("Protocol connection setup failed")
            return False
        
        try:
            # Check if it's a single scenario
            if test_name in self.scenarios_config:
                result = self.execute_single_scenario(test_name)
                success = result.success
                logger.info(f"Single scenario result: {result.message}")
                
            # Check if it's a sequence
            elif test_name in self.sequences_config:
                result = self.execute_scenario_sequence(test_name)
                success = result.success
                logger.info(f"Sequence result: {result.message}")
                
            else:
                logger.error(f"Test '{test_name}' not found in scenarios or sequences")
                success = False
            
            return success
            
        finally:
            self.cleanup_connection()