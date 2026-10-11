#!/usr/bin/env python3
"""
Documentation Generator for nur.h

This script automatically generates API documentation from the nur.h header file,
extracting comments, function signatures, macros, types, and usage examples.
"""

import re
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple

class APIElement:
    """Represents an API element (function, macro, type, constant)"""
    def __init__(self, name: str, element_type: str, signature: str = "", 
                 description: str = "", examples: List[str] = None):
        self.name = name
        self.element_type = element_type  # 'macro', 'type', 'function', 'constant'
        self.signature = signature
        self.description = description
        self.examples = examples or []
        self.parameters = []
        self.return_info = ""

def extract_comment_block(lines: List[str], start_idx: int) -> Tuple[str, int]:
    """Extract doc comment block starting at start_idx."""
    comment_lines = []
    i = start_idx
    
    if lines[i].strip().startswith('//'):
        while i < len(lines) and lines[i].strip().startswith('//'):
            comment_lines.append(lines[i].strip()[2:].strip())
            i += 1
        return '\n'.join(comment_lines), i
    elif lines[i].strip().startswith('/*'):
        # Skip section banner comments like /*========...
        if '====' in lines[i]:
            while i < len(lines) and not lines[i].strip().endswith('*/'):
                i += 1
            return "", i + 1
        
        in_comment = True
        first_line = lines[i].strip()[2:]
        if first_line.endswith('*/'):
            return first_line[:-2].strip(), i + 1
        
        if first_line.strip() and not first_line.strip().startswith('*'):
            comment_lines.append(first_line.strip())
        i += 1
        
        while i < len(lines) and in_comment:
            line = lines[i].strip()
            if line.endswith('*/'):
                content = line[:-2].strip()
                if content.startswith('*'):
                    content = content[1:].strip()
                if content:
                    comment_lines.append(content)
                in_comment = False
                i += 1
            else:
                if line.startswith('*'):
                    line = line[1:].strip()
                comment_lines.append(line)
                i += 1
        
        return '\n'.join(comment_lines), i
    
    return "", start_idx

def extract_api_elements(header_content: str) -> List[APIElement]:
    """Extract public API elements from nur.h."""
    lines = header_content.split('\n')
    elements = []
    i = 0
    
    while i < len(lines):
        line = lines[i].strip()
        if not line:
            i += 1
            continue
        
        comment = ""
        if line.startswith('/*') or line.startswith('//'):
            comment, i = extract_comment_block(lines, i)
            # Advance to declaration line while keeping comment
            while i < len(lines) and not lines[i].strip():
                i += 1
            if i >= len(lines):
                break
            line = lines[i].strip()
        
        # Macros
        if line.startswith('#define'):
            match = re.match(r'#define\s+([A-Za-z0-9_]+)(?:\(([^)]*)\))?(?:\s+(.*))?', line)
            if match:
                macro_name = match.group(1)
                macro_params = match.group(2)
                macro_body = match.group(3) if match.group(3) else ""
                
                # Filter out internal/private/guard macros
                if (not macro_name.startswith('_') and 
                    not macro_name.endswith('_H') and 
                    not macro_name.startswith('NUR_LOC_') and
                    not macro_name.startswith('NUR_INTERNAL_') and
                    not macro_name.startswith('nur_internal_')):
                    
                    sig = f"#define {macro_name}"
                    if macro_params is not None:
                        sig += f"({macro_params})"
                    if macro_body and not macro_body.endswith('\\'):
                        sig += f" {macro_body.strip()}"
                    
                    element = APIElement(macro_name, "macro", sig, comment)
                    elements.append(element)
            i += 1
            continue
        
        # Typedef structs
        elif line.startswith('typedef struct'):
            match = re.search(r'typedef\s+struct(?:\s+[A-Za-z0-9_\[\]]+)?\s*\{.*?\}\s*([A-Za-z0-9_]+)\s*;', line)
            if not match:
                # Multi-line typedef
                typedef_lines = [line]
                j = i + 1
                while j < len(lines) and not typedef_lines[-1].endswith(';'):
                    typedef_lines.append(lines[j].strip())
                    j += 1
                typedef_text = ' '.join(typedef_lines)
                m = re.search(r'typedef\s+struct(?:\s+[A-Za-z0-9_\[\]]+)?\s*(?:[A-Za-z0-9_]+)?\s*\{.*?\}\s*([A-Za-z0-9_]+)\s*;', typedef_text)
                if m:
                    type_name = m.group(1)
                    element = APIElement(type_name, "type", f"typedef struct {type_name} {type_name};", comment)
                    elements.append(element)
                i = j
                continue
            else:
                type_name = match.group(1)
                element = APIElement(type_name, "type", f"typedef struct {type_name} {type_name};", comment)
                elements.append(element)
                i += 1
                continue
        
        # Enums
        elif re.match(r'^\s*NUR_E_[A-Za-z0-9_]+\s*[,=]', line):
            m = re.match(r'^\s*(NUR_E_[A-Za-z0-9_]+)\s*(?:=\s*([0-9]+))?', line)
            if m:
                code_name = m.group(1)
                sig = f"{code_name}" + (f" = {m.group(2)}" if m.group(2) else "")
                element = APIElement(code_name, "constant", sig, comment)
                elements.append(element)
            i += 1
            continue
        
        # Functions: NUR_INLINE ... name(...)
        elif 'nur_' in line and '(' in line and not line.startswith('#'):
            # Collect full function header up to '{' or ';'
            func_lines = [line]
            j = i + 1
            while j < len(lines) and '{' not in func_lines[-1] and ';' not in func_lines[-1]:
                func_lines.append(lines[j].strip())
                j += 1
            full_header = ' '.join(func_lines)
            sig_match = re.search(r'(?:NUR_NODISCARD\s+)?(?:NUR_INLINE\s+)?([A-Za-z0-9_* ]+?)\s+(nur_[A-Za-z0-9_]+)\s*\(([^)]*)\)', full_header)
            if sig_match:
                ret_type = sig_match.group(1).strip()
                func_name = sig_match.group(2).strip()
                params = sig_match.group(3).strip()
                
                # Filter out internal functions
                if not func_name.startswith('nur_internal_'):
                    sig = f"{ret_type} {func_name}({params})"
                    element = APIElement(func_name, "function", sig, comment)
                    element.return_info = ret_type
                    elements.append(element)
            i = j
            continue
        
        i += 1
    
    return elements

def generate_markdown_docs(elements: List[APIElement]) -> str:
    """Generate Markdown API documentation."""
    docs = []
    docs.append("# nur.h API Reference\n")
    docs.append("Comprehensive, auto-generated API documentation for `nur.h` (Author: Irene Al-Caligo).\n")
    
    by_type = {}
    for el in elements:
        by_type.setdefault(el.element_type, []).append(el)
    
    section_order = ['type', 'macro', 'function', 'constant']
    section_titles = {
        'type': 'Types & Structures',
        'macro': 'Macros & Propagation',
        'function': 'Functions & Utilities',
        'constant': 'Error Codes'
    }
    
    for sec in section_order:
        if sec not in by_type:
            continue
        items = sorted(by_type[sec], key=lambda x: x.name)
        docs.append(f"## {section_titles[sec]}\n")
        for item in items:
            docs.append(f"### `{item.name}`\n")
            docs.append("```c")
            docs.append(item.signature)
            docs.append("```\n")
            if item.description:
                docs.append(item.description)
                docs.append("")
            docs.append("---\n")
    
    return "\n".join(docs)

def main():
    parser = argparse.ArgumentParser(description='Generate documentation for nur.h')
    parser.add_argument('header_file', help='Path to nur.h header file')
    parser.add_argument('--format', choices=['markdown', 'html'], default='markdown',
                        help='Output format (default: markdown)')
    parser.add_argument('--output', help='Output file path (default: stdout)')
    args = parser.parse_args()
    
    content = Path(args.header_file).read_text(encoding='utf-8')
    elements = extract_api_elements(content)
    docs = generate_markdown_docs(elements)
    
    if args.output:
        Path(args.output).write_text(docs, encoding='utf-8')
        print(f"Documentation written to: {args.output} ({len(elements)} items documented)")
    else:
        print(docs)
    return 0

if __name__ == '__main__':
    sys.exit(main())