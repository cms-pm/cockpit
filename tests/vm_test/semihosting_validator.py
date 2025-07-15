#!/usr/bin/env python3
"""
SemihostingValidator - Semihosting output capture and validation
Validates program output against YAML-defined patterns and requirements
"""

import re
import time
from typing import Dict, Any, List, Optional, Union
from enum import Enum


class PatternType(Enum):
    """Types of pattern matching for semihosting output"""
    CONTAINS = "contains"
    NOT_CONTAINS = "not_contains"
    REGEX = "pattern"
    SEQUENCE = "sequence"
    EXACT = "exact"


class SemihostingValidator:
    """
    Validates semihosting output against configuration patterns
    
    Supports:
    - Content matching (contains/not_contains)
    - Regex pattern matching
    - Sequence validation (ordered output)
    - Exact matching
    """
    
    def __init__(self):
        self.supported_patterns = {
            'contains': self._validate_contains,
            'not_contains': self._validate_not_contains,
            'pattern': self._validate_regex,
            'sequence': self._validate_sequence,
            'exact': self._validate_exact
        }
    
    def validate_output(self, output: str, checks: List[Dict[str, Any]]) -> Dict[str, Any]:
        """
        Validate semihosting output against check configuration
        
        Args:
            output: Captured semihosting output
            checks: List of validation checks from YAML
            
        Returns:
            Dictionary with validation results
        """
        if not checks:
            return {
                'passed': True,
                'message': 'No semihosting checks configured',
                'checks': []
            }
        
        check_results = []
        all_passed = True
        
        for check in checks:
            result = self._validate_single_check(output, check)
            check_results.append(result)
            
            if not result['passed']:
                all_passed = False
        
        # Create summary message
        passed_count = sum(1 for r in check_results if r['passed'])
        total_count = len(check_results)
        
        if all_passed:
            message = f"All {total_count} semihosting checks passed"
        else:
            message = f"{passed_count}/{total_count} semihosting checks passed"
        
        return {
            'passed': all_passed,
            'message': message,
            'checks': check_results
        }
    
    def _validate_single_check(self, output: str, check: Dict[str, Any]) -> Dict[str, Any]:
        """
        Validate a single check against output
        
        Args:
            output: Semihosting output
            check: Single check configuration
            
        Returns:
            Dictionary with check result
        """
        # Determine check type
        check_type = None
        check_value = None
        
        for pattern_type in self.supported_patterns.keys():
            if pattern_type in check:
                check_type = pattern_type
                check_value = check[pattern_type]
                break
        
        if check_type is None:
            return {
                'passed': False,
                'type': 'unknown',
                'message': 'Unknown check type',
                'details': str(check)
            }
        
        # Execute validation
        try:
            validator = self.supported_patterns[check_type]
            result = validator(output, check_value)
            
            # Add metadata
            result['type'] = check_type
            result['expected'] = check_value
            
            return result
            
        except Exception as e:
            return {
                'passed': False,
                'type': check_type,
                'message': f'Check execution error: {e}',
                'expected': check_value,
                'details': str(e)
            }
    
    def _validate_contains(self, output: str, expected: str) -> Dict[str, Any]:
        """Validate that output contains expected string"""
        found = expected in output
        
        return {
            'passed': found,
            'message': f"{'Found' if found else 'Missing'} required string: '{expected}'",
            'details': self._get_context_around_match(output, expected) if found else None
        }
    
    def _validate_not_contains(self, output: str, forbidden: str) -> Dict[str, Any]:
        """Validate that output does not contain forbidden string"""
        found = forbidden in output
        
        return {
            'passed': not found,
            'message': f"{'Found forbidden' if found else 'Correctly absent'} string: '{forbidden}'",
            'details': self._get_context_around_match(output, forbidden) if found else None
        }
    
    def _validate_regex(self, output: str, pattern: str) -> Dict[str, Any]:
        """Validate output against regex pattern"""
        try:
            match = re.search(pattern, output, re.MULTILINE | re.DOTALL)
            found = match is not None
            
            return {
                'passed': found,
                'message': f"Regex pattern {'matched' if found else 'not found'}: '{pattern}'",
                'details': {
                    'match': match.group(0) if match else None,
                    'groups': match.groups() if match else None
                }
            }
            
        except re.error as e:
            return {
                'passed': False,
                'message': f"Invalid regex pattern: {e}",
                'details': str(e)
            }
    
    def _validate_sequence(self, output: str, sequence: List[str]) -> Dict[str, Any]:
        """Validate that strings appear in specified order"""
        if not sequence:
            return {
                'passed': True,
                'message': 'Empty sequence - trivially valid',
                'details': []
            }
        
        output_lines = output.split('\n')
        sequence_progress = []
        current_sequence_index = 0
        
        for line_num, line in enumerate(output_lines):
            if current_sequence_index < len(sequence):
                expected = sequence[current_sequence_index]
                
                if expected in line:
                    sequence_progress.append({
                        'expected': expected,
                        'found_at_line': line_num + 1,
                        'line_content': line.strip()
                    })
                    current_sequence_index += 1
        
        all_found = current_sequence_index == len(sequence)
        
        return {
            'passed': all_found,
            'message': f"Sequence progress: {current_sequence_index}/{len(sequence)} items found in order",
            'details': {
                'progress': sequence_progress,
                'missing': sequence[current_sequence_index:] if not all_found else []
            }
        }
    
    def _validate_exact(self, output: str, expected: str) -> Dict[str, Any]:
        """Validate exact match of entire output"""
        # Normalize whitespace for comparison
        normalized_output = output.strip()
        normalized_expected = expected.strip()
        
        exact_match = normalized_output == normalized_expected
        
        return {
            'passed': exact_match,
            'message': f"Exact match: {'success' if exact_match else 'failed'}",
            'details': {
                'output_length': len(normalized_output),
                'expected_length': len(normalized_expected),
                'diff_preview': self._create_diff_preview(normalized_output, normalized_expected)
            }
        }
    
    def _get_context_around_match(self, output: str, search_term: str, 
                                  context_lines: int = 2) -> Dict[str, Any]:
        """Get context lines around a match"""
        output_lines = output.split('\n')
        
        for i, line in enumerate(output_lines):
            if search_term in line:
                start = max(0, i - context_lines)
                end = min(len(output_lines), i + context_lines + 1)
                
                return {
                    'line_number': i + 1,
                    'matched_line': line.strip(),
                    'context': [
                        {
                            'line_num': start + j + 1,
                            'content': output_lines[start + j].strip(),
                            'is_match': start + j == i
                        }
                        for j in range(end - start)
                    ]
                }
        
        return {'line_number': None, 'matched_line': None, 'context': []}
    
    def _create_diff_preview(self, actual: str, expected: str, max_chars: int = 100) -> str:
        """Create a preview of differences between actual and expected"""
        if actual == expected:
            return "No differences"
        
        # Simple character-by-character comparison
        diff_chars = []
        min_len = min(len(actual), len(expected))
        
        for i in range(min_len):
            if actual[i] != expected[i]:
                diff_chars.append(f"@{i}: got '{actual[i]}', expected '{expected[i]}'")
                if len(diff_chars) >= 3:  # Limit to first 3 differences
                    break
        
        if len(actual) != len(expected):
            diff_chars.append(f"Length: got {len(actual)}, expected {len(expected)}")
        
        return " | ".join(diff_chars)
    
    def create_validation_summary(self, checks: List[Dict[str, Any]]) -> str:
        """
        Create human-readable summary of semihosting validation
        
        Args:
            checks: List of check results
            
        Returns:
            String summary of validation results
        """
        if not checks:
            return "No semihosting validation performed"
        
        passed_count = sum(1 for check in checks if check['passed'])
        total_count = len(checks)
        
        summary_lines = [f"Semihosting validation: {passed_count}/{total_count} checks passed"]
        
        # Add details for failed checks
        for check in checks:
            if not check['passed']:
                summary_lines.append(f"  ✗ {check['type']}: {check['message']}")
        
        return "\n".join(summary_lines)