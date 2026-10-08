#!/usr/bin/env python3
"""
API Compatibility Checker for nur.h

This script checks for API compatibility issues between versions of nur.h,
ensuring that changes don't break existing code.
"""

import re
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Set, Tuple

def extract_api_elements(header_content: str) -> Dict[str, Set[str]]:
    """
    Extract API elements (macros, types, functions) from header content.
    """
    api = {
        'macros': set(),
        'types': set(),
        'functions': set(),
        'constants': set()
    }
    
    lines = header_content.split('\n')
    
    for line in lines:
        line = line.strip()
        
        # Skip comments and empty lines
        if not line or line.startswith('//') or line.startswith('/*'):
            continue
            
        # Extract #define macros
        if line.startswith('#define'):
            match = re.match(r'#define\s+([A-Z_][A-Z0-9_]*)', line)
            if match:
                macro_name = match.group(1)
                # Skip internal macros (those starting with underscore)
                if not macro_name.startswith('_'):
                    api['macros'].add(macro_name)
        
        # Extract typedef types
        elif 'typedef' in line:
            # Handle typedef struct/enum/union
            match = re.search(r'typedef\s+(?:struct|enum|union)\s*\{[^}]*\}\s*([A-Za-z_][A-Za-z0-9_]*)', line, re.MULTILINE | re.DOTALL)
            if not match:
                # Handle simple typedefs
                match = re.search(r'typedef\s+[^;]+\s+([A-Za-z_][A-Za-z0-9_]*)\s*;', line)
            if match:
                api['types'].add(match.group(1))
        
        # Extract function declarations
        elif re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*\s+[a-zA-Z_][a-zA-Z0-9_]*\s*\(', line):
            match = re.match(r'^[^\s]+\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(', line)
            if match:
                func_name = match.group(1)
                # Skip internal functions
                if not func_name.startswith('_'):
                    api['functions'].add(func_name)
        
        # Extract enum constants
        elif re.match(r'\s*[A-Z_][A-Z0-9_]*\s*[,=]', line):
            match = re.match(r'\s*([A-Z_][A-Z0-9_]*)\s*[,=]', line)
            if match:
                const_name = match.group(1)
                if not const_name.startswith('_'):
                    api['constants'].add(const_name)
    
    return api

def compare_apis(old_api: Dict[str, Set[str]], new_api: Dict[str, Set[str]]) -> Dict[str, List[str]]:
    """
    Compare two API versions and return differences.
    """
    changes = {
        'removed_macros': list(old_api['macros'] - new_api['macros']),
        'added_macros': list(new_api['macros'] - old_api['macros']),
        'removed_types': list(old_api['types'] - new_api['types']),
        'added_types': list(new_api['types'] - old_api['types']),
        'removed_functions': list(old_api['functions'] - new_api['functions']),
        'added_functions': list(new_api['functions'] - old_api['functions']),
        'removed_constants': list(old_api['constants'] - new_api['constants']),
        'added_constants': list(new_api['constants'] - old_api['constants'])
    }
    
    return changes

def check_breaking_changes(changes: Dict[str, List[str]]) -> List[str]:
    """
    Identify breaking changes that would affect API compatibility.
    """
    breaking_changes = []
    
    if changes['removed_macros']:
        breaking_changes.extend([f"BREAKING: Removed macro '{macro}'" for macro in changes['removed_macros']])
    
    if changes['removed_types']:
        breaking_changes.extend([f"BREAKING: Removed type '{type_}'" for type_ in changes['removed_types']])
    
    if changes['removed_functions']:
        breaking_changes.extend([f"BREAKING: Removed function '{func}'" for func in changes['removed_functions']])
    
    if changes['removed_constants']:
        breaking_changes.extend([f"BREAKING: Removed constant '{const}'" for const in changes['removed_constants']])
    
    return breaking_changes

def generate_report(changes: Dict[str, List[str]], breaking_changes: List[str]) -> str:
    """
    Generate a formatted compatibility report.
    """
    report = []
    report.append("=== API Compatibility Report ===\n")
    
    if breaking_changes:
        report.append("🚨 BREAKING CHANGES DETECTED:")
        for change in breaking_changes:
            report.append(f"  • {change}")
        report.append("")
    
    # Additions (non-breaking)
    additions = []
    if changes['added_macros']:
        additions.extend([f"  + Macro: {macro}" for macro in sorted(changes['added_macros'])])
    if changes['added_types']:
        additions.extend([f"  + Type: {type_}" for type_ in sorted(changes['added_types'])])
    if changes['added_functions']:
        additions.extend([f"  + Function: {func}" for func in sorted(changes['added_functions'])])
    if changes['added_constants']:
        additions.extend([f"  + Constant: {const}" for const in sorted(changes['added_constants'])])
    
    if additions:
        report.append("✅ API ADDITIONS (Non-breaking):")
        report.extend(additions)
        report.append("")
    
    if not breaking_changes and not additions:
        report.append("✅ No API changes detected.")
    
    return "\n".join(report)

def main():
    parser = argparse.ArgumentParser(description='Check nur.h API compatibility between versions')
    parser.add_argument('old_header', help='Path to old version of nur.h')
    parser.add_argument('new_header', help='Path to new version of nur.h')
    parser.add_argument('--fail-on-breaking', action='store_true', 
                       help='Exit with error code if breaking changes are detected')
    parser.add_argument('--output', help='Write report to file instead of stdout')
    
    args = parser.parse_args()
    
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    if hasattr(sys.stderr, 'reconfigure'):
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    
    # Read header files
    try:
        old_content = Path(args.old_header).read_text(encoding='utf-8')
        new_content = Path(args.new_header).read_text(encoding='utf-8')
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Error reading files: {e}", file=sys.stderr)
        return 1
    
    # Extract API elements
    old_api = extract_api_elements(old_content)
    new_api = extract_api_elements(new_content)
    
    # Compare APIs
    changes = compare_apis(old_api, new_api)
    breaking_changes = check_breaking_changes(changes)
    
    # Generate report
    report = generate_report(changes, breaking_changes)
    
    # Output report
    if args.output:
        try:
            Path(args.output).write_text(report, encoding='utf-8')
            print(f"Report written to: {args.output}")
        except Exception as e:
            print(f"Error writing report: {e}", file=sys.stderr)
            return 1
    else:
        print(report)
    
    # Exit with error if breaking changes detected and --fail-on-breaking is set
    if args.fail_on_breaking and breaking_changes:
        return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main())