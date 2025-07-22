#!/usr/bin/env python3
"""
Workspace Builder for ComponentVM Hardware Tests
Creates isolated PlatformIO workspaces for reliable test execution

This module implements the core workspace isolation that solves the build
conflicts and reliability issues of the legacy test system.
"""

import os
import shutil
import yaml
from pathlib import Path

class WorkspaceBuilder:
    """Creates isolated test workspaces with complete PlatformIO environments"""
    
    def __init__(self, base_dir=None):
        if base_dir is None:
            # Default to tests/ directory relative to this script
            script_dir = Path(__file__).parent
            self.base_dir = script_dir.parent
        else:
            self.base_dir = Path(base_dir)
            
        self.base_project_dir = self.base_dir / "base_project"
        self.test_registry_dir = self.base_dir / "test_registry"
        self.workspaces_dir = self.base_dir / "active_workspaces"
        self.lib_dir = self.base_dir.parent / "lib"  # Project lib directory
        
    def create_test_workspace(self, test_name):
        """
        Create isolated workspace for specified test
        
        Args:
            test_name: Name of test (e.g., 'pc6_led_focused')
            
        Returns:
            Path to created workspace directory
            
        Raises:
            FileNotFoundError: If test source or dependencies not found
            ValueError: If test not found in catalog
        """
        print(f"Creating workspace for test: {test_name}")
        
        # Load test metadata
        test_metadata = self._load_test_metadata(test_name)
        
        # Create workspace directory
        workspace_path = self.workspaces_dir / test_name
        
        # Clean workspace if it exists
        if workspace_path.exists():
            print(f"   Cleaning existing workspace: {workspace_path}")
            shutil.rmtree(workspace_path)
            
        workspace_path.mkdir(parents=True, exist_ok=True)
        print(f"   Created workspace directory: {workspace_path}")
        
        # Generate PlatformIO configuration from template
        self._generate_platformio_config(test_name, test_metadata, workspace_path)
        
        # Create source directory and copy test files
        src_dir = workspace_path / "src"
        src_dir.mkdir(exist_ok=True)
        
        # Copy test source file
        self._copy_test_source(test_name, test_metadata, src_dir)
        
        # Copy dependencies if any
        self._copy_dependencies(test_metadata, src_dir)
        
        # Generate main.c from template
        self._generate_main_c(test_name, test_metadata, src_dir)
        
        # Create library symlink
        self._create_lib_symlink(workspace_path)
        
        print(f"   ✓ Workspace created successfully: {workspace_path}")
        return workspace_path
        
    def _load_test_metadata(self, test_name):
        """Load test metadata from catalog"""
        catalog_path = self.test_registry_dir / "test_catalog.yaml"
        
        if not catalog_path.exists():
            raise FileNotFoundError(f"Test catalog not found: {catalog_path}")
            
        with open(catalog_path, 'r') as f:
            catalog = yaml.safe_load(f)
            
        if test_name not in catalog.get('tests', {}):
            raise ValueError(f"Test '{test_name}' not found in catalog")
            
        return catalog['tests'][test_name]
    
    def load_test_metadata(self, test_name):
        """Public method to load test metadata from catalog"""
        return self._load_test_metadata(test_name)
        
    def _copy_base_project(self, workspace_path):
        """Generate PlatformIO configuration from template"""
        # Legacy fallback: check for old platformio.ini
        base_platformio = self.base_project_dir / "platformio.ini"
        template_platformio = self.base_project_dir / "platformio_template.ini"
        
        if template_platformio.exists():
            print(f"   Using template-based PlatformIO configuration")
            # Template system not implemented in this method - will be called separately
        elif base_platformio.exists():
            # Legacy fallback
            workspace_platformio = workspace_path / "platformio.ini"
            shutil.copy2(base_platformio, workspace_platformio)
            print(f"   Copied legacy base PlatformIO config")
        else:
            raise FileNotFoundError(f"Neither template nor base platformio.ini found")
        
    def _copy_test_source(self, test_name, test_metadata, src_dir):
        """Copy test source file to workspace"""
        source_file = test_metadata['source']
        test_source_path = self.test_registry_dir / "src" / source_file
        workspace_source = src_dir / source_file
        
        if not test_source_path.exists():
            raise FileNotFoundError(f"Test source not found: {test_source_path}")
            
        # Create directory structure if needed (for subdirectory sources)
        workspace_source.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(test_source_path, workspace_source)
        print(f"   Copied test source: {source_file}")
        
    def _copy_dependencies(self, test_metadata, src_dir):
        """Copy test dependencies to workspace"""
        dependencies = test_metadata.get('dependencies', [])
        
        for dep in dependencies:
            dep_source = self.test_registry_dir / "src" / dep
            dep_target = src_dir / dep
            
            if not dep_source.exists():
                raise FileNotFoundError(f"Dependency not found: {dep_source}")
                
            # Create directory structure if needed
            dep_target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(dep_source, dep_target)
            print(f"   Copied dependency: {dep}")
            
    def _generate_main_c(self, test_name, test_metadata, src_dir):
        """Generate main.c from template"""
        template_path = self.base_project_dir / "src_template" / "main_template.c"
        main_c_path = src_dir / "main.c"
        
        if not template_path.exists():
            raise FileNotFoundError(f"Main template not found: {template_path}")
            
        # Read template
        with open(template_path, 'r') as f:
            template_content = f.read()
            
        # Determine test function name (convention: run_<test_name>_main)
        test_function = f"run_{test_name}_main"
        
        # Determine platform configuration
        platform_config = self._get_platform_config(test_metadata)
        
        # Replace template placeholders with platform-aware substitutions
        main_content = template_content.replace("{{TEST_NAME}}", test_name)
        main_content = main_content.replace("{{TEST_FUNCTION}}", test_function)
        main_content = main_content.replace("{{PLATFORM_NAME}}", platform_config.get('name', 'stm32g4'))
        
        # Add platform-specific template validation
        if platform_config.get('requires_test_interface', False):
            self._validate_platform_test_interface_availability(platform_config)
        
        # Write generated main.c
        with open(main_c_path, 'w') as f:
            f.write(main_content)
            
        print(f"   Generated main.c calling: {test_function}")
        print(f"   Platform configuration: {platform_config.get('name', 'stm32g4')}")
        
        # Copy platform test implementation if needed
        self._copy_platform_test_implementation(src_dir, platform_config)
        
    def _get_platform_config(self, test_metadata):
        """Extract platform configuration from test metadata"""
        # Default to STM32G4 for backward compatibility
        default_config = {
            'name': 'stm32g4',
            'interface': 'stm32g4_uart_test',
            'includes': ['test_platform/platform_test_interface.h'],
            'requires_test_interface': True,
            'implementation_file': 'platform_test_stm32g4.c'
        }
        
        # Future: support platform selection from test metadata
        # platform = test_metadata.get('platform', 'stm32g4')
        
        return default_config
    
    def _validate_platform_test_interface_availability(self, platform_config):
        """Validate that platform test interface components are available"""
        # Check if platform test interface header exists
        interface_header = self.base_dir.parent / "lib" / "vm_cockpit" / "src" / "test_platform" / "platform_test_interface.h"
        if not interface_header.exists():
            raise FileNotFoundError(f"Platform test interface header not found: {interface_header}")
            
        # Check if platform implementation exists
        impl_name = platform_config.get('implementation_file', 'platform_test_stm32g4.c')
        impl_template = self.base_project_dir / "src_template" / impl_name
        if not impl_template.exists():
            raise FileNotFoundError(f"Platform test implementation template not found: {impl_template}")
            
        print(f"   ✓ Platform test interface validated for: {platform_config['name']}")
    
    def _copy_platform_test_implementation(self, src_dir, platform_config):
        """Copy platform test implementation source file with platform awareness"""
        impl_name = platform_config.get('implementation_file', 'platform_test_stm32g4.c')
        platform_test_template = self.base_project_dir / "src_template" / impl_name
        platform_test_dest = src_dir / impl_name
        
        if platform_test_template.exists():
            shutil.copy2(platform_test_template, platform_test_dest)
            print(f"   Copied platform test implementation: {impl_name}")
            print(f"   Platform interface: {platform_config.get('interface', 'unknown')}")
        else:
            if platform_config.get('requires_test_interface', False):
                raise FileNotFoundError(f"Required platform test template not found: {platform_test_template}")
            else:
                print(f"   Info: Platform test interface not required for this test")
        
    def _create_lib_symlink(self, workspace_path):
        """Create symlink to shared libraries"""
        workspace_lib = workspace_path / "lib"
        
        # Remove existing lib if present
        if workspace_lib.exists():
            if workspace_lib.is_symlink():
                workspace_lib.unlink()
            else:
                shutil.rmtree(workspace_lib)
                
        # Create symlink to project lib directory
        try:
            workspace_lib.symlink_to(self.lib_dir, target_is_directory=True)
            print(f"   Created lib symlink to: {self.lib_dir}")
        except OSError as e:
            print(f"   Warning: Could not create lib symlink: {e}")
            # Fallback: copy lib directory
            shutil.copytree(self.lib_dir, workspace_lib)
            print(f"   Copied lib directory as fallback")
            
    def list_available_tests(self):
        """List all available tests from catalog"""
        catalog_path = self.test_registry_dir / "test_catalog.yaml"
        
        if not catalog_path.exists():
            return []
            
        with open(catalog_path, 'r') as f:
            catalog = yaml.safe_load(f)
            
        return list(catalog.get('tests', {}).keys())
        
    def cleanup_workspace(self, test_name):
        """Clean up workspace for specified test"""
        workspace_path = self.workspaces_dir / test_name
        
        if workspace_path.exists():
            shutil.rmtree(workspace_path)
            print(f"Cleaned up workspace: {workspace_path}")
        else:
            print(f"Workspace not found: {workspace_path}")
            
    def cleanup_all_workspaces(self):
        """Clean up all active workspaces"""
        if not self.workspaces_dir.exists():
            return
            
        for workspace in self.workspaces_dir.iterdir():
            if workspace.is_dir() and workspace.name != '.gitignore':
                shutil.rmtree(workspace)
                print(f"Cleaned up workspace: {workspace}")

    def _generate_platformio_config(self, test_name, test_metadata, workspace_path):
        """Generate platformio.ini from template with test-specific configuration"""
        
        # Build configuration context
        config_context = self._build_configuration_context(test_metadata)
        
        # Load template
        template_path = self.base_project_dir / "platformio_template.ini"
        if not template_path.exists():
            # Fallback to legacy copy method
            self._copy_base_project(workspace_path)
            return
            
        with open(template_path, 'r') as f:
            template_content = f.read()
        
        # Apply template substitutions
        for placeholder, value in config_context.items():
            template_content = template_content.replace(f"{{{{{placeholder}}}}}", value)
        
        # Write generated platformio.ini
        platformio_path = workspace_path / "platformio.ini"
        with open(platformio_path, 'w') as f:
            f.write(template_content)
        
        print(f"   Generated platformio.ini with configuration: {list(config_context.keys())}")

    def _build_configuration_context(self, test_metadata):
        """Build template substitution context from test metadata"""
        context = {
            'BUILD_FLAGS': '',
            'DEBUG_COMMANDS': '',
            'LIB_DEPS': ''
        }
        
        # Configure semihosting
        self._configure_semihosting(context, test_metadata)
        
        # Configure custom build flags
        self._configure_custom_build_flags(context, test_metadata)
        
        # Configure optimization (future extensibility)
        self._configure_optimization(context, test_metadata)
        
        # Configure memory model (future extensibility)
        self._configure_memory_model(context, test_metadata)
        
        # Configure test framework (future extensibility) 
        self._configure_test_framework(context, test_metadata)
        
        return context

    def _configure_semihosting(self, context, test_metadata):
        """Configure semihosting-related build settings"""
        semihosting_enabled = test_metadata.get('semihosting', True)
        
        if not semihosting_enabled:
            # Add disable flag to build flags
            context['BUILD_FLAGS'] += '\t-DDISABLE_SEMIHOSTING\n'
            print(f"   Semihosting disabled for this test")
        else:
            # Add debug commands for semihosting
            context['DEBUG_COMMANDS'] = '''debug_init_cmds = 
\tmonitor arm semihosting enable
\tmonitor reset halt'''
            print(f"   Semihosting enabled for this test")

    def _configure_custom_build_flags(self, context, test_metadata):
        """Add custom build flags from test metadata"""
        custom_flags = test_metadata.get('build_flags', [])
        
        for flag in custom_flags:
            context['BUILD_FLAGS'] += f'\t{flag}\n'
            
        if custom_flags:
            print(f"   Added {len(custom_flags)} custom build flags")

    def _configure_optimization(self, context, test_metadata):
        """Configure optimization level (future extensibility)"""
        optimization = test_metadata.get('optimization', None)
        
        if optimization == 'size':
            context['BUILD_FLAGS'] += '\t-Os\n'
            print(f"   Optimization: size (-Os)")
        elif optimization == 'speed':
            context['BUILD_FLAGS'] += '\t-O2\n'
            print(f"   Optimization: speed (-O2)")
        elif optimization == 'debug':
            context['BUILD_FLAGS'] += '\t-Og\n'
            print(f"   Optimization: debug (-Og)")
        # Default -O0 already in template

    def _configure_memory_model(self, context, test_metadata):
        """Configure memory model settings (future extensibility)"""
        memory_model = test_metadata.get('memory_model', None)
        
        if memory_model == 'minimal':
            context['BUILD_FLAGS'] += '\t-DCOMPONENTVM_MINIMAL_MEMORY\n'
            print(f"   Memory model: minimal")
        elif memory_model == 'extended':
            context['BUILD_FLAGS'] += '\t-DCOMPONENTVM_EXTENDED_MEMORY\n'
            print(f"   Memory model: extended")

    def _configure_test_framework(self, context, test_metadata):
        """Configure test framework settings (future extensibility)"""
        test_framework = test_metadata.get('test_framework', None)
        
        if test_framework == 'unity':
            context['LIB_DEPS'] += '\n\tunity'
            context['BUILD_FLAGS'] += '\t-DUSE_UNITY_FRAMEWORK\n'
            print(f"   Test framework: Unity")
        elif test_framework == 'catch2':
            context['LIB_DEPS'] += '\n\tcatch2'
            context['BUILD_FLAGS'] += '\t-DUSE_CATCH2_FRAMEWORK\n'
            print(f"   Test framework: Catch2")


if __name__ == "__main__":
    # Simple CLI for testing workspace builder
    import sys
    
    if len(sys.argv) < 2:
        print("Usage: python workspace_builder.py <test_name>")
        sys.exit(1)
        
    test_name = sys.argv[1]
    builder = WorkspaceBuilder()
    
    try:
        workspace = builder.create_test_workspace(test_name)
        print(f"Workspace created successfully: {workspace}")
    except Exception as e:
        print(f"Error creating workspace: {e}")
        sys.exit(1)