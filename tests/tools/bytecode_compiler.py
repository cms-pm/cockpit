#!/usr/bin/env python3
"""
vm_compiler Python Wrapper for Golden Triangle Integration
Phase 4.9.2: Golden Triangle Bytecode Infrastructure

Provides clean interface to componentvm_compiler for test automation,
enabling Golden Triangle tests to compile guest ArduinoC programs to bytecode
for execution on actual CockpitVM hardware.

Author: cms-pm
Date: 2025-09-19
Phase: 4.9.2
"""

import subprocess
import os
import tempfile
from pathlib import Path
from typing import NamedTuple, Optional, List, Dict
import logging
import shutil

# Configure logging for Golden Triangle integration
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class CompileResult(NamedTuple):
    """Result of bytecode compilation operation"""
    success: bool
    bytecode_path: Optional[str]
    error_message: Optional[str]
    compiler_output: str
    instruction_count: int
    string_count: int

class BytecodeMetadata(NamedTuple):
    """Metadata extracted from compiled bytecode"""
    instruction_count: int
    string_count: int
    bytecode_size_bytes: int
    source_file: str
    compilation_time: float

class BytecodeCompiler:
    """
    Python wrapper for componentvm_compiler with Golden Triangle integration

    Provides automated ArduinoC → bytecode compilation for guest programs
    that will execute on CockpitVM hardware with Golden Triangle validation.
    """

    def __init__(self, verbose: bool = False):
        """
        Initialize bytecode compiler

        Args:
            verbose: Enable verbose logging output
        """
        self.verbose = verbose

        # Path to built componentvm_compiler (relative to project root)
        project_root = Path(__file__).parent.parent.parent
        self.compiler_path = project_root / "lib" / "vm_compiler" / "tools" / "build" / "componentvm_compiler"

        # Validate compiler availability
        if not self.compiler_path.exists():
            raise FileNotFoundError(f"componentvm_compiler not found at: {self.compiler_path}")

        if not os.access(self.compiler_path, os.X_OK):
            raise PermissionError(f"componentvm_compiler not executable: {self.compiler_path}")

        logger.info(f"BytecodeCompiler initialized with compiler: {self.compiler_path}")

    def compile_arduino_c(self, source_file: str, output_dir: Optional[str] = None) -> CompileResult:
        """
        Compile ArduinoC source to CockpitVM bytecode

        Args:
            source_file: Path to .c source file
            output_dir: Output directory (default: same as source)

        Returns:
            CompileResult with success status, paths, and metadata
        """
        import time

        start_time = time.time()
        source_path = Path(source_file)

        # Validate source file
        if not source_path.exists():
            return CompileResult(
                success=False,
                bytecode_path=None,
                error_message=f"Source file not found: {source_file}",
                compiler_output="",
                instruction_count=0,
                string_count=0
            )

        if source_path.suffix.lower() not in ['.c', '.cpp']:
            return CompileResult(
                success=False,
                bytecode_path=None,
                error_message=f"Invalid source file extension: {source_path.suffix} (expected .c or .cpp)",
                compiler_output="",
                instruction_count=0,
                string_count=0
            )

        try:
            # Determine output file path
            if output_dir:
                output_dir_path = Path(output_dir)
                output_dir_path.mkdir(parents=True, exist_ok=True)
                expected_bytecode_path = output_dir_path / f"{source_path.stem}.bin"
            else:
                expected_bytecode_path = source_path.parent / f"{source_path.stem}.bin"

            # Execute componentvm_compiler
            if self.verbose:
                logger.info(f"Compiling {source_file} with componentvm_compiler...")

            result = subprocess.run(
                [str(self.compiler_path), str(source_path)],
                capture_output=True,
                text=True,
                timeout=30  # 30 second timeout for compilation
            )

            compilation_time = time.time() - start_time

            # Check for compilation success
            if result.returncode != 0:
                return CompileResult(
                    success=False,
                    bytecode_path=None,
                    error_message=f"Compilation failed with exit code {result.returncode}",
                    compiler_output=result.stdout + result.stderr,
                    instruction_count=0,
                    string_count=0
                )

            # componentvm_compiler creates .bin file in source directory
            default_output_path = source_path.parent / f"{source_path.stem}.bin"

            if not default_output_path.exists():
                return CompileResult(
                    success=False,
                    bytecode_path=None,
                    error_message="Compilation succeeded but bytecode file not found",
                    compiler_output=result.stdout + result.stderr,
                    instruction_count=0,
                    string_count=0
                )

            # Move bytecode file to desired output location if needed
            final_bytecode_path = expected_bytecode_path
            if default_output_path != expected_bytecode_path:
                shutil.move(str(default_output_path), str(final_bytecode_path))

            # Extract metadata from bytecode file
            metadata = self._extract_bytecode_metadata(final_bytecode_path, source_file, compilation_time)

            if self.verbose:
                logger.info(f"Compilation successful: {metadata.instruction_count} instructions, "
                          f"{metadata.string_count} strings, {metadata.bytecode_size_bytes} bytes")

            return CompileResult(
                success=True,
                bytecode_path=str(final_bytecode_path),
                error_message=None,
                compiler_output=result.stdout + result.stderr,
                instruction_count=metadata.instruction_count,
                string_count=metadata.string_count
            )

        except subprocess.TimeoutExpired:
            return CompileResult(
                success=False,
                bytecode_path=None,
                error_message="Compilation timed out after 30 seconds",
                compiler_output="",
                instruction_count=0,
                string_count=0
            )
        except Exception as e:
            return CompileResult(
                success=False,
                bytecode_path=None,
                error_message=f"Unexpected error during compilation: {str(e)}",
                compiler_output="",
                instruction_count=0,
                string_count=0
            )

    def compile_string_to_bytecode(self, arduino_c_code: str,
                                  output_dir: Optional[str] = None,
                                  temp_filename: str = "guest_program") -> CompileResult:
        """
        Compile ArduinoC source code string to bytecode

        Useful for Golden Triangle tests that generate ArduinoC code programmatically.

        Args:
            arduino_c_code: ArduinoC source code as string
            output_dir: Output directory for bytecode (default: temp directory)
            temp_filename: Base filename for temporary source file

        Returns:
            CompileResult with compilation status and bytecode path
        """
        # Create temporary source file
        with tempfile.NamedTemporaryFile(mode='w', suffix='.c', delete=False,
                                       prefix=f"{temp_filename}_") as temp_file:
            temp_file.write(arduino_c_code)
            temp_source_path = temp_file.name

        try:
            # Compile the temporary source file
            result = self.compile_arduino_c(temp_source_path, output_dir)

            return result

        finally:
            # Clean up temporary source file
            try:
                os.unlink(temp_source_path)
            except OSError:
                pass  # Ignore cleanup errors

    def validate_bytecode_file(self, bytecode_path: str) -> bool:
        """
        Validate that a bytecode file has correct format

        Args:
            bytecode_path: Path to bytecode file to validate

        Returns:
            True if bytecode file is valid, False otherwise
        """
        try:
            bytecode_file = Path(bytecode_path)
            if not bytecode_file.exists():
                return False

            # Check minimum file size (at least 8 bytes for header)
            if bytecode_file.stat().st_size < 8:
                return False

            # Try to read header
            with open(bytecode_file, 'rb') as f:
                instruction_count_bytes = f.read(4)
                string_count_bytes = f.read(4)

                if len(instruction_count_bytes) != 4 or len(string_count_bytes) != 4:
                    return False

            return True

        except Exception:
            return False

    def get_compiler_info(self) -> Dict[str, str]:
        """
        Get information about the componentvm_compiler

        Returns:
            Dictionary with compiler information
        """
        return {
            "compiler_path": str(self.compiler_path),
            "compiler_exists": self.compiler_path.exists(),
            "compiler_executable": os.access(self.compiler_path, os.X_OK) if self.compiler_path.exists() else False,
            "version": "Phase 4.9.2 CockpitVM Bytecode Compiler"
        }

    def _extract_bytecode_metadata(self, bytecode_path: Path, source_file: str,
                                 compilation_time: float) -> BytecodeMetadata:
        """
        Extract metadata from compiled bytecode file

        Args:
            bytecode_path: Path to bytecode file
            source_file: Original source file path
            compilation_time: Time taken for compilation

        Returns:
            BytecodeMetadata with extracted information
        """
        try:
            file_size = bytecode_path.stat().st_size

            with open(bytecode_path, 'rb') as f:
                # Read header: instruction count (2 bytes) + string count (2 bytes) + padding (4 bytes)
                instruction_count_bytes = f.read(2)
                string_count_bytes = f.read(2)
                padding_bytes = f.read(4)  # Skip padding/additional header

                instruction_count = int.from_bytes(instruction_count_bytes, byteorder='little')
                string_count = int.from_bytes(string_count_bytes, byteorder='little')

            return BytecodeMetadata(
                instruction_count=instruction_count,
                string_count=string_count,
                bytecode_size_bytes=file_size,
                source_file=source_file,
                compilation_time=compilation_time
            )

        except Exception as e:
            logger.warning(f"Could not extract bytecode metadata: {e}")
            return BytecodeMetadata(
                instruction_count=0,
                string_count=0,
                bytecode_size_bytes=0,
                source_file=source_file,
                compilation_time=compilation_time
            )


# Golden Triangle Integration Helper Functions

def create_simple_gpio_test(pin_number: int = 13, test_name: str = "guest_gpio_test") -> str:
    """
    Generate simple ArduinoC GPIO test program for Golden Triangle validation

    Args:
        pin_number: GPIO pin number to test (default: 13 for PC6)
        test_name: Name for the test program

    Returns:
        ArduinoC source code string
    """
    return f'''/*
 * {test_name} - Generated Golden Triangle GPIO Test
 * Tests pin {pin_number} configuration and digital output operations
 */

void main() {{
    // Configure pin {pin_number} as output
    pinMode({pin_number}, 1);  // OUTPUT = 1

    // Test digital write HIGH
    digitalWrite({pin_number}, 1);  // HIGH = 1

    // Brief delay
    delay(100);

    // Test digital write LOW
    digitalWrite({pin_number}, 0);  // LOW = 0

    printf("GPIO Pin {pin_number} test complete\\n");
}}
'''

def create_printf_routing_test(test_message: str = "Hello from guest bytecode!") -> str:
    """
    Generate ArduinoC printf routing test for Golden Triangle validation

    Args:
        test_message: Message to print (will be routed via IOController)

    Returns:
        ArduinoC source code string
    """
    return f'''/*
 * Printf Routing Test - Generated Golden Triangle Test
 * Tests automatic printf routing via IOController (debugger detection)
 */

void main() {{
    printf("PRINTF_ROUTING_TEST_START\\n");
    printf("{test_message}\\n");
    printf("PRINTF_ROUTING_TEST_END\\n");
}}
'''


if __name__ == "__main__":
    """
    Command-line interface for bytecode compiler
    Usage: python3 bytecode_compiler.py <source_file.c> [output_dir]
    """
    import sys

    if len(sys.argv) < 2:
        print("Usage: python3 bytecode_compiler.py <source_file.c> [output_dir]")
        sys.exit(1)

    source_file = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else None

    compiler = BytecodeCompiler(verbose=True)

    print(f"Golden Triangle Bytecode Compiler (Phase 4.9.2)")
    print(f"Compiler info: {compiler.get_compiler_info()}")
    print()

    result = compiler.compile_arduino_c(source_file, output_dir)

    if result.success:
        print(f"✅ Compilation successful!")
        print(f"   Bytecode: {result.bytecode_path}")
        print(f"   Instructions: {result.instruction_count}")
        print(f"   Strings: {result.string_count}")
        sys.exit(0)
    else:
        print(f"❌ Compilation failed: {result.error_message}")
        if result.compiler_output:
            print(f"Compiler output:\n{result.compiler_output}")
        sys.exit(1)