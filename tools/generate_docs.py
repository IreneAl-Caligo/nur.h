#!/usr/bin/env python3
"""
Documentation Generator for nur.h

This script automatically generates API documentation from the nur.h header file,
extracting comments, function signatures, and usage examples.
"""

import re
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple

class APIElement:
    """Represents an API element (function, macro, type, etc.)"""
    
    def __init__(self, name: str, element_type: str, signature: str = "", 
                 description: str = "", examples: List[str] = None):
        self.name = name
        self.element_type = element_type  # 'function', 'macro', 'type', 'constant'
        self.signature = signature
        self.description = description
        self.examples = examples or []
        self.parameters = []
        self.return_info = ""
        self.see_also = []

def extract_comment_block(lines: List[str], start_idx: int) -> Tuple[str, int]:
    """
    Extract a comment block starting at the given index.
    Returns (comment_text, next_line_index)
    """
    comment_lines = []
    i = start_idx
    
    # Handle single-line comment
    if lines[i].strip().startswith('//'):
        while i < len(lines) and lines[i].strip().startswith('//'):
            comment_lines.append(lines[i].strip()[2:].strip())
            i += 1
        return '\\n'.join(comment_lines), i
    
    # Handle multi-line comment
    elif lines[i].strip().startswith('/*'):
        in_comment = True
        first_line = lines[i].strip()[2:]
        if first_line.endswith('*/'):
            return first_line[:-2].strip(), i + 1
        
        comment_lines.append(first_line)
        i += 1
        
        while i < len(lines) and in_comment:
            line = lines[i].strip()
            if line.endswith('*/'):
                comment_lines.append(line[:-2].strip())
                in_comment = False
                i += 1
            else:
                # Remove leading * from comment lines
                if line.startswith('*'):
                    line = line[1:].strip()
                comment_lines.append(line)
                i += 1
        
        return '\\n'.join(comment_lines), i
    
    return "", start_idx

def parse_function_signature(signature: str) -> Tuple[str, List[Dict], str]:
    """
    Parse function signature to extract return type, parameters, and function name.
    """
    # Simple regex to match function signature
    match = re.match(r'^(.+?)\s+(\w+)\s*\(([^)]*)\)', signature.strip())
    if not match:
        return "", [], ""
    
    return_type = match.group(1).strip()
    func_name = match.group(2).strip()
    params_str = match.group(3).strip()
    
    parameters = []
    if params_str and params_str != 'void':
        param_parts = params_str.split(',')
        for param in param_parts:
            param = param.strip()
            if param:
                # Try to split type and name
                parts = param.split()
                if len(parts) >= 2:
                    param_type = ' '.join(parts[:-1])
                    param_name = parts[-1]
                else:
                    param_type = param
                    param_name = ""
                
                parameters.append({
                    'type': param_type,
                    'name': param_name,
                    'description': ""
                })
    
    return return_type, parameters, func_name

def extract_api_elements(header_content: str) -> List[APIElement]:
    """
    Extract API elements from the header file content.
    """
    lines = header_content.split('\\n')
    elements = []
    i = 0
    
    while i < len(lines):
        line = lines[i].strip()
        
        # Skip empty lines
        if not line:
            i += 1
            continue
        
        # Check for comment block
        comment = ""
        if line.startswith('/*') or line.startswith('//'):
            comment, i = extract_comment_block(lines, i)
            if i >= len(lines):
                break
            line = lines[i].strip()
        
        # Extract macro definitions
        if line.startswith('#define'):
            match = re.match(r'#define\\s+(\\w+)(?:\\s*\\(([^)]*)\\))?(?:\\s+(.*))?', line)
            if match:
                macro_name = match.group(1)
                macro_params = match.group(2) if match.group(2) else ""
                macro_body = match.group(3) if match.group(3) else ""
                
                # Skip internal macros
                if not macro_name.startswith('_'):
                    signature = f"#define {macro_name}"
                    if macro_params:
                        signature += f"({macro_params})"
                    if macro_body:
                        signature += f" {macro_body}"
                    
                    element = APIElement(macro_name, "macro", signature, comment)
                    elements.append(element)
        
        # Extract typedef declarations
        elif line.startswith('typedef'):
            # Try to find the end of the typedef
            typedef_lines = [line]
            j = i + 1
            while j < len(lines) and not typedef_lines[-1].rstrip().endswith(';'):
                typedef_lines.append(lines[j].strip())
                j += 1
            
            typedef_text = ' '.join(typedef_lines)
            
            # Extract type name
            match = re.search(r'typedef\\s+.*?\\s+(\\w+)\\s*;', typedef_text)
            if match:
                type_name = match.group(1)
                element = APIElement(type_name, "type", typedef_text, comment)
                elements.append(element)
            
            i = j
            continue
        
        # Extract function declarations
        elif re.match(r'^\\w+.*\\w+\\s*\\(', line):
            # Try to find complete function signature
            func_lines = [line]
            j = i + 1
            while j < len(lines) and not func_lines[-1].rstrip().endswith(';'):
                func_lines.append(lines[j].strip())
                j += 1
            
            signature = ' '.join(func_lines).replace(';', '')
            return_type, parameters, func_name = parse_function_signature(signature)
            
            if func_name and not func_name.startswith('_'):
                element = APIElement(func_name, "function", signature, comment)
                element.parameters = parameters
                element.return_info = return_type
                elements.append(element)
            
            i = j
            continue
        
        # Extract enum values
        elif re.match(r'^\\s*\\w+\\s*[,=]', line) and not line.startswith('#'):
            match = re.match(r'^\\s*(\\w+)\\s*[,=]', line)
            if match:
                const_name = match.group(1)
                if not const_name.startswith('_'):
                    element = APIElement(const_name, "constant", line, comment)
                    elements.append(element)
        
        i += 1
    
    return elements

def generate_markdown_docs(elements: List[APIElement]) -> str:
    """
    Generate markdown documentation from API elements.
    """
    docs = []
    docs.append("# nur.h API Reference\\n")
    docs.append("Auto-generated API documentation for nur.h library.\\n")
    
    # Group elements by type
    by_type = {}
    for element in elements:
        if element.element_type not in by_type:
            by_type[element.element_type] = []
        by_type[element.element_type].append(element)
    
    # Generate sections
    section_order = ['macro', 'type', 'function', 'constant']
    section_titles = {
        'macro': 'Macros',
        'type': 'Types',
        'function': 'Functions', 
        'constant': 'Constants'
    }
    
    for section_type in section_order:
        if section_type not in by_type:
            continue
        
        section_elements = sorted(by_type[section_type], key=lambda x: x.name)
        docs.append(f"## {section_titles[section_type]}\\n")
        
        for element in section_elements:
            docs.append(f"### {element.name}\\n")
            
            # Signature
            docs.append("```c")
            docs.append(element.signature)
            docs.append("```\\n")
            
            # Description
            if element.description:
                docs.append(element.description)
                docs.append("")
            
            # Parameters (for functions)
            if element.element_type == 'function' and element.parameters:
                docs.append("**Parameters:**\\n")
                for param in element.parameters:
                    param_desc = param['description'] if param['description'] else "No description"
                    docs.append(f"- `{param['name']}` ({param['type']}): {param_desc}")
                docs.append("")
            
            # Return value (for functions)
            if element.element_type == 'function' and element.return_info:
                docs.append(f"**Returns:** `{element.return_info}`\\n")
            
            # Examples
            if element.examples:
                docs.append("**Example:**\\n")
                for example in element.examples:
                    docs.append("```c")
                    docs.append(example)
                    docs.append("```\\n")
            
            docs.append("---\\n")
    
    return "\\n".join(docs)

def generate_html_docs(elements: List[APIElement]) -> str:
    """
    Generate HTML documentation from API elements.
    """
    html = []
    html.append("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>nur.h API Reference</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 1200px; margin: 0 auto; padding: 20px; }
        .header { border-bottom: 2px solid #333; margin-bottom: 30px; }
        .section { margin: 30px 0; }
        .api-element { border: 1px solid #ddd; margin: 20px 0; padding: 20px; border-radius: 5px; }
        .signature { background: #f5f5f5; padding: 10px; font-family: monospace; border-left: 4px solid #0066cc; }
        .description { margin: 15px 0; line-height: 1.6; }
        .parameters { margin: 15px 0; }
        .parameter { margin: 5px 0; }
        .example { background: #f9f9f9; padding: 15px; font-family: monospace; border: 1px solid #eee; }
        h1, h2, h3 { color: #333; }
        .toc { background: #f8f9fa; padding: 20px; border-radius: 5px; margin: 20px 0; }
        .toc ul { list-style-type: none; padding-left: 20px; }
        .toc a { text-decoration: none; color: #0066cc; }
    </style>
</head>
<body>""")
    
    html.append('<div class="header">')
    html.append('<h1>nur.h API Reference</h1>')
    html.append('<p>Auto-generated API documentation for nur.h library.</p>')
    html.append('</div>')
    
    # Table of contents
    by_type = {}
    for element in elements:
        if element.element_type not in by_type:
            by_type[element.element_type] = []
        by_type[element.element_type].append(element)
    
    html.append('<div class="toc">')
    html.append('<h2>Table of Contents</h2>')
    html.append('<ul>')
    
    section_order = ['macro', 'type', 'function', 'constant']
    section_titles = {
        'macro': 'Macros',
        'type': 'Types',
        'function': 'Functions',
        'constant': 'Constants'
    }
    
    for section_type in section_order:
        if section_type not in by_type:
            continue
        html.append(f'<li><a href="#{section_type}s">{section_titles[section_type]}</a>')
        html.append('<ul>')
        section_elements = sorted(by_type[section_type], key=lambda x: x.name)
        for element in section_elements:
            html.append(f'<li><a href="#{element.name}">{element.name}</a></li>')
        html.append('</ul></li>')
    
    html.append('</ul>')
    html.append('</div>')
    
    # Generate sections
    for section_type in section_order:
        if section_type not in by_type:
            continue
        
        section_elements = sorted(by_type[section_type], key=lambda x: x.name)
        html.append(f'<div class="section">')
        html.append(f'<h2 id="{section_type}s">{section_titles[section_type]}</h2>')
        
        for element in section_elements:
            html.append('<div class="api-element">')
            html.append(f'<h3 id="{element.name}">{element.name}</h3>')
            
            # Signature
            html.append('<div class="signature">')
            html.append(element.signature.replace('<', '&lt;').replace('>', '&gt;'))
            html.append('</div>')
            
            # Description
            if element.description:
                html.append('<div class="description">')
                # Simple markdown to HTML conversion
                desc = element.description.replace('\\n\\n', '</p><p>').replace('\\n', '<br>')
                if not desc.startswith('<p>'):
                    desc = '<p>' + desc
                if not desc.endswith('</p>'):
                    desc = desc + '</p>'
                html.append(desc)
                html.append('</div>')
            
            # Parameters
            if element.element_type == 'function' and element.parameters:
                html.append('<div class="parameters">')
                html.append('<strong>Parameters:</strong>')
                for param in element.parameters:
                    param_desc = param['description'] if param['description'] else "No description"
                    html.append(f'<div class="parameter"><code>{param["name"]}</code> ({param["type"]}): {param_desc}</div>')
                html.append('</div>')
            
            # Return value
            if element.element_type == 'function' and element.return_info:
                html.append(f'<div><strong>Returns:</strong> <code>{element.return_info}</code></div>')
            
            # Examples
            if element.examples:
                html.append('<div><strong>Example:</strong></div>')
                for example in element.examples:
                    html.append('<div class="example">')
                    html.append(example.replace('<', '&lt;').replace('>', '&gt;'))
                    html.append('</div>')
            
            html.append('</div>')
        
        html.append('</div>')
    
    html.append('</body></html>')
    
    return '\\n'.join(html)

def main():
    parser = argparse.ArgumentParser(description='Generate documentation for nur.h')
    parser.add_argument('header_file', help='Path to nur.h header file')
    parser.add_argument('--format', choices=['markdown', 'html'], default='markdown',
                       help='Output format (default: markdown)')
    parser.add_argument('--output', help='Output file path (default: stdout)')
    
    args = parser.parse_args()
    
    try:
        header_content = Path(args.header_file).read_text(encoding='utf-8')
    except FileNotFoundError as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"Error reading header file: {e}", file=sys.stderr)
        return 1
    
    # Extract API elements
    elements = extract_api_elements(header_content)
    
    # Generate documentation
    if args.format == 'html':
        docs = generate_html_docs(elements)
    else:
        docs = generate_markdown_docs(elements)
    
    # Output documentation
    if args.output:
        try:
            Path(args.output).write_text(docs, encoding='utf-8')
            print(f"Documentation written to: {args.output}")
        except Exception as e:
            print(f"Error writing documentation: {e}", file=sys.stderr)
            return 1
    else:
        print(docs)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())