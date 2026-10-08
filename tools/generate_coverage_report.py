#!/usr/bin/env python3
"""
Test Coverage Report Generator for nur.h

This script analyzes test coverage and generates detailed reports
showing which parts of the API are tested and which need more coverage.
"""

import re
import sys
import json
import argparse
from pathlib import Path
from typing import Dict, List, Set, Tuple

def extract_test_functions(test_content: str) -> Set[str]:
    """
    Extract test function names from test files.
    """
    test_functions = set()
    
    # Match test functions (functions starting with test_ or ending with _test)
    test_pattern = r'(?:static\s+)?void\s+(test_\w+|\w+_test)\s*\('
    matches = re.findall(test_pattern, test_content, re.MULTILINE)
    
    for match in matches:
        test_functions.add(match)
    
    return test_functions

def extract_tested_apis(test_content: str) -> Set[str]:
    """
    Extract API elements that are being tested (mentioned in test code).
    """
    tested_apis = set()
    
    # Common nur.h API patterns
    patterns = [
        r'(NUR_\w+)\s*\(',  # Macro calls
        r'(NurResult_\w+)',  # Result types
        r'(NurOption_\w+)',  # Option types
        r'(NUR_E_\w+)',      # Error codes
        r'(nur_\w+)\s*\(',   # Utility functions
    ]
    
    for pattern in patterns:
        matches = re.findall(pattern, test_content, re.MULTILINE)
        tested_apis.update(matches)
    
    return tested_apis

def extract_api_elements(header_content: str) -> Dict[str, Set[str]]:
    """
    Extract all API elements from the header file.
    """
    api = {
        'macros': set(),
        'result_types': set(),
        'option_types': set(),
        'error_codes': set(),
        'functions': set()
    }
    
    lines = header_content.split('\n')
    
    for line in lines:
        line = line.strip()
        
        if not line or line.startswith('//') or line.startswith('/*'):
            continue
        
        # Extract macros
        if line.startswith('#define'):
            match = re.match(r'#define\s+(NUR_\w+)', line)
            if match:
                api['macros'].add(match.group(1))
        
        # Extract result types
        elif 'NurResult_' in line:
            matches = re.findall(r'(NurResult_\w+)', line)
            api['result_types'].update(matches)
        
        # Extract option types
        elif 'NurOption_' in line:
            matches = re.findall(r'(NurOption_\w+)', line)
            api['option_types'].update(matches)
        
        # Extract error codes
        elif re.match(r'\s*NUR_E_\w+', line):
            match = re.match(r'\s*(NUR_E_\w+)', line)
            if match:
                api['error_codes'].add(match.group(1))
        
        # Extract functions
        elif re.match(r'^[a-zA-Z_]\w*\s+nur_\w+\s*\(', line):
            match = re.match(r'^[^\s]+\s+(nur_\w+)\s*\(', line)
            if match:
                api['functions'].add(match.group(1))
    
    return api

def calculate_coverage(api_elements: Dict[str, Set[str]], tested_apis: Set[str]) -> Dict[str, Dict]:
    """
    Calculate coverage statistics for each API category.
    """
    coverage = {}
    
    for category, apis in api_elements.items():
        if not apis:
            coverage[category] = {
                'total': 0,
                'tested': 0,
                'coverage_percent': 100.0,
                'tested_items': [],
                'untested_items': []
            }
            continue
        
        tested_items = [api for api in apis if api in tested_apis]
        untested_items = [api for api in apis if api not in tested_apis]
        
        coverage[category] = {
            'total': len(apis),
            'tested': len(tested_items),
            'coverage_percent': (len(tested_items) / len(apis)) * 100 if apis else 100,
            'tested_items': sorted(tested_items),
            'untested_items': sorted(untested_items)
        }
    
    return coverage

def generate_report(coverage: Dict[str, Dict], test_functions: Set[str]) -> str:
    """
    Generate a formatted coverage report.
    """
    report = []
    report.append("=== Test Coverage Report for nur.h ===\n")
    
    # Overall statistics
    total_apis = sum(cat['total'] for cat in coverage.values())
    total_tested = sum(cat['tested'] for cat in coverage.values())
    overall_coverage = (total_tested / total_apis) * 100 if total_apis > 0 else 100
    
    report.append(f"📊 Overall Coverage: {overall_coverage:.1f}% ({total_tested}/{total_apis} APIs tested)")
    report.append(f"🧪 Total Test Functions: {len(test_functions)}\n")
    
    # Category breakdown
    for category, stats in coverage.items():
        if stats['total'] == 0:
            continue
            
        category_name = category.replace('_', ' ').title()
        coverage_emoji = "✅" if stats['coverage_percent'] >= 90 else "⚠️" if stats['coverage_percent'] >= 70 else "❌"
        
        report.append(f"{coverage_emoji} {category_name}: {stats['coverage_percent']:.1f}% ({stats['tested']}/{stats['total']})")
        
        if stats['untested_items']:
            report.append("   Untested:")
            for item in stats['untested_items'][:10]:  # Show first 10
                report.append(f"     • {item}")
            if len(stats['untested_items']) > 10:
                report.append(f"     ... and {len(stats['untested_items']) - 10} more")
        
        report.append("")
    
    # Recommendations
    report.append("📝 RECOMMENDATIONS:")
    
    low_coverage_categories = [cat for cat, stats in coverage.items() 
                              if stats['coverage_percent'] < 80 and stats['total'] > 0]
    
    if low_coverage_categories:
        report.append("   Priority areas for additional testing:")
        for category in low_coverage_categories:
            category_name = category.replace('_', ' ').title()
            report.append(f"     • {category_name} ({coverage[category]['coverage_percent']:.1f}% covered)")
    else:
        report.append("   • Excellent coverage! Consider adding edge case tests.")
    
    # Test function analysis
    if test_functions:
        report.append(f"\n📋 TEST FUNCTIONS ({len(test_functions)}):")
        for func in sorted(test_functions)[:10]:  # Show first 10
            report.append(f"   • {func}")
        if len(test_functions) > 10:
            report.append(f"   ... and {len(test_functions) - 10} more")
    
    return "\n".join(report)

def generate_json_report(coverage: Dict[str, Dict], test_functions: Set[str]) -> Dict:
    """
    Generate a JSON format report for machine consumption.
    """
    total_apis = sum(cat['total'] for cat in coverage.values())
    total_tested = sum(cat['tested'] for cat in coverage.values())
    overall_coverage = (total_tested / total_apis) * 100 if total_apis > 0 else 100
    
    return {
        'overall_coverage_percent': round(overall_coverage, 2),
        'total_apis': total_apis,
        'total_tested': total_tested,
        'total_test_functions': len(test_functions),
        'categories': coverage,
        'test_functions': sorted(list(test_functions)),
        'recommendations': [
            category for category, stats in coverage.items()
            if stats['coverage_percent'] < 80 and stats['total'] > 0
        ]
    }

def main():
    parser = argparse.ArgumentParser(description='Generate test coverage report for nur.h')
    parser.add_argument('header_file', help='Path to nur.h header file')
    parser.add_argument('test_files', nargs='+', help='Path to test files')
    parser.add_argument('--json', action='store_true', help='Output JSON format')
    parser.add_argument('--output', help='Write report to file instead of stdout')
    parser.add_argument('--min-coverage', type=float, default=80.0,
                       help='Minimum acceptable coverage percentage (default: 80.0)')
    
    args = parser.parse_args()
    
    if hasattr(sys.stdout, 'reconfigure'):
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    if hasattr(sys.stderr, 'reconfigure'):
        sys.stderr.reconfigure(encoding='utf-8', errors='replace')
    
    try:
        # Read header file
        header_content = Path(args.header_file).read_text(encoding='utf-8')
        
        # Read and combine all test files
        test_content = ""
        for test_file in args.test_files:
            test_content += Path(test_file).read_text(encoding='utf-8') + "\n"
        
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Error reading files: {e}", file=sys.stderr)
        return 1
    
    # Extract API elements and test information
    api_elements = extract_api_elements(header_content)
    tested_apis = extract_tested_apis(test_content)
    test_functions = extract_test_functions(test_content)
    
    # Calculate coverage
    coverage = calculate_coverage(api_elements, tested_apis)
    
    # Generate report
    if args.json:
        report_data = generate_json_report(coverage, test_functions)
        report = json.dumps(report_data, indent=2)
    else:
        report = generate_report(coverage, test_functions)
    
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
    
    # Check minimum coverage requirement
    total_apis = sum(cat['total'] for cat in coverage.values())
    total_tested = sum(cat['tested'] for cat in coverage.values())
    overall_coverage = (total_tested / total_apis) * 100 if total_apis > 0 else 100
    
    if overall_coverage < args.min_coverage:
        print(f"\nWarning: Coverage {overall_coverage:.1f}% is below minimum {args.min_coverage}%", 
              file=sys.stderr)
        return 1
    
    return 0

if __name__ == '__main__':
    sys.exit(main())