"""
ValidationResult - Rich result objects with debugging context
"""

from typing import Dict, Any, Optional
from enum import Enum

class ValidationStatus(Enum):
    PASSED = "PASSED"
    FAILED = "FAILED"
    ERROR = "ERROR"
    SKIPPED = "SKIPPED"

class ValidationResult:
    """Rich validation result with debugging context and dual-pass support"""
    
    def __init__(self, test_name: str):
        self.test_name = test_name
        self.final_status = ValidationStatus.SKIPPED
        self.message = ""
        self.authority = "supplemental"
        self.execution_time_ms = 0
        self.pass_results: Dict[str, Any] = {}
        self.context: Dict[str, Any] = {}
        self.diagnostics: Dict[str, Any] = {}
        
    def set_final_status(self, status: ValidationStatus):
        """Set the final validation status"""
        self.final_status = status
    
    def set_message(self, message: str):
        """Set the validation message"""
        self.message = message
    
    def set_authority(self, authority: str):
        """Set validation authority level"""
        self.authority = authority
    
    def set_execution_time(self, time_ms: int):
        """Set execution time in milliseconds"""
        self.execution_time_ms = time_ms
    
    def add_pass_result(self, pass_name: str, result: Dict[str, Any]):
        """Add result from a validation pass"""
        self.pass_results[pass_name] = result
    
    def add_context(self, key: str, value: Any):
        """Add debugging context information"""
        self.context[key] = value
    
    def add_memory_dump(self, start_address: int, data: bytes):
        """Add memory dump context for debugging"""
        self.context['memory_dump'] = {
            'start_address': start_address,
            'data': data.hex() if isinstance(data, bytes) else str(data),
            'size': len(data) if isinstance(data, bytes) else 0
        }
    
    def add_register_interpretation(self, register_name: str, bit_fields: Dict[str, Any]):
        """Add register bit field interpretation"""
        self.context['register_interpretation'] = {
            'name': register_name,
            'fields': bit_fields
        }
    
    def add_diagnostic(self, key: str, value: Any):
        """Add diagnostic information"""
        self.diagnostics[key] = value
    
    @property
    def is_passed(self) -> bool:
        return self.final_status == ValidationStatus.PASSED
    
    @property
    def is_failed(self) -> bool:
        return self.final_status == ValidationStatus.FAILED
    
    @property
    def is_error(self) -> bool:
        return self.final_status == ValidationStatus.ERROR
    
    @property
    def is_skipped(self) -> bool:
        return self.final_status == ValidationStatus.SKIPPED
    
    def get_pass_result(self, pass_name: str) -> Optional[Dict[str, Any]]:
        """Get result from a specific validation pass"""
        return self.pass_results.get(pass_name)
    
    def __str__(self) -> str:
        status_emoji = {
            ValidationStatus.PASSED: "✅",
            ValidationStatus.FAILED: "❌", 
            ValidationStatus.ERROR: "🚨",
            ValidationStatus.SKIPPED: "⏭️"
        }
        
        result = f"{status_emoji[self.final_status]} {self.test_name}"
        if self.message:
            result += f": {self.message}"
        
        if self.execution_time_ms > 0:
            result += f" ({self.execution_time_ms}ms)"
            
        return result
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for JSON serialization"""
        return {
            'test_name': self.test_name,
            'status': self.final_status.value,
            'message': self.message,
            'authority': self.authority,
            'execution_time_ms': self.execution_time_ms,
            'pass_results': self.pass_results,
            'context': self.context,
            'diagnostics': self.diagnostics
        }
    
    def create_summary(self) -> str:
        """Create a human-readable summary of the validation result"""
        lines = [f"Test: {self.test_name}"]
        lines.append(f"Status: {self.final_status.value}")
        lines.append(f"Message: {self.message}")
        lines.append(f"Authority: {self.authority}")
        
        if self.execution_time_ms > 0:
            lines.append(f"Execution Time: {self.execution_time_ms}ms")
        
        # Add pass results
        for pass_name, pass_result in self.pass_results.items():
            lines.append(f"{pass_name.title()} Pass: {pass_result.get('status', 'UNKNOWN')}")
            if 'message' in pass_result:
                lines.append(f"  {pass_result['message']}")
        
        # Add diagnostics if available
        if self.diagnostics:
            lines.append("Diagnostics:")
            for key, value in self.diagnostics.items():
                lines.append(f"  {key}: {value}")
        
        return "\n".join(lines)