#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Detailed Code Analysis for USB_HID_CAN_BRIDGE
Checks code patterns, potential issues, and provides recommendations.
"""

import os
import sys
import re
from pathlib import Path
from collections import defaultdict

# Fix Windows console encoding
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

class CodeAnalyzer:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.issues = []
        self.suggestions = []
        self.patterns_found = defaultdict(list)
        
    def analyze_code(self):
        """Run detailed code analysis"""
        print("=" * 80)
        print("DETAILED CODE ANALYSIS")
        print("=" * 80)
        print()
        
        firmware_dir = self.project_root / 'epic_can_logger'
        if not firmware_dir.exists():
            print("Error: firmware directory not found")
            return
        
        # Analyze main firmware files
        ino_files = list(firmware_dir.glob('*.ino'))
        for ino_file in ino_files:
            print(f"Analyzing: {ino_file.name}")
            self.analyze_file(ino_file)
            print()
        
        # Analyze C++ modules
        cpp_files = list(firmware_dir.glob('*.cpp'))
        for cpp_file in cpp_files:
            print(f"Analyzing: {cpp_file.name}")
            self.analyze_file(cpp_file)
            print()
        
        # Print findings
        self.print_findings()
    
    def analyze_file(self, file_path):
        """Analyze a single file for patterns and issues"""
        try:
            content = file_path.read_text(encoding='utf-8', errors='ignore')
            lines = content.split('\n')
            
            # Check for common patterns
            self.check_time_budgeting(content, file_path.name)
            self.check_error_handling(content, file_path.name)
            self.check_debug_macros(content, file_path.name)
            self.check_memory_management(content, file_path.name)
            self.check_priority_scheduling(content, file_path.name)
            self.check_comments_and_docs(content, file_path.name)
            self.check_function_complexity(content, file_path.name)
            self.check_includes(content, file_path.name)
            
        except Exception as e:
            self.issues.append(f"Error analyzing {file_path.name}: {e}")
    
    def check_time_budgeting(self, content, filename):
        """Check for time budgeting patterns"""
        if 'millis()' in content and ('MAX_TIME' in content or 'time_budget' in content.lower()):
            self.patterns_found['time_budgeting'].append(filename)
            print("  ✓ Uses time budgeting (good practice)")
        elif 'millis()' in content and 'loop()' in content:
            self.suggestions.append(f"{filename}: Consider adding time budgets for non-critical operations")
    
    def check_error_handling(self, content, filename):
        """Check for error handling patterns"""
        error_patterns = ['if.*error', 'catch', 'return.*false', 'return.*NULL', 'return.*nullptr']
        has_error_handling = any(re.search(pattern, content, re.IGNORECASE) for pattern in error_patterns)
        
        if has_error_handling:
            self.patterns_found['error_handling'].append(filename)
            print("  ✓ Contains error handling")
        else:
            self.suggestions.append(f"{filename}: Consider adding more error handling")
    
    def check_debug_macros(self, content, filename):
        """Check for debug macro usage"""
        if 'DEBUG_' in content:
            self.patterns_found['debug_macros'].append(filename)
            print("  ✓ Uses debug macros (configurable)")
        elif 'Serial.print' in content and filename.endswith('.ino'):
            self.suggestions.append(f"{filename}: Consider using DEBUG_ macros instead of direct Serial.print")
    
    def check_memory_management(self, content, filename):
        """Check for memory management issues"""
        # Check for potential memory leaks
        if 'malloc' in content and 'free' not in content:
            self.issues.append(f"{filename}: Uses malloc() but no corresponding free() found")
        elif 'new ' in content and 'delete ' not in content and 'delete[]' not in content:
            self.issues.append(f"{filename}: Uses new but no corresponding delete found")
        else:
            print("  ✓ Memory management appears safe")
    
    def check_priority_scheduling(self, content, filename):
        """Check for priority-based scheduling"""
        if 'PRIORITY' in content or 'priority' in content.lower():
            self.patterns_found['priority_scheduling'].append(filename)
            print("  ✓ Uses priority-based scheduling")
    
    def check_comments_and_docs(self, content, filename):
        """Check code documentation"""
        lines = content.split('\n')
        code_lines = [l for l in lines if l.strip() and not l.strip().startswith('//') and not l.strip().startswith('/*')]
        comment_lines = [l for l in lines if l.strip().startswith('//') or '/*' in l or '*/' in l]
        
        if len(code_lines) > 0:
            comment_ratio = len(comment_lines) / len(code_lines)
            if comment_ratio < 0.1:
                self.suggestions.append(f"{filename}: Low comment ratio ({comment_ratio:.1%}), consider adding more documentation")
            else:
                print(f"  ✓ Good documentation ({comment_ratio:.1%} comment ratio)")
    
    def check_function_complexity(self, content, filename):
        """Check for complex functions"""
        # Simple heuristic: functions with many lines
        lines = content.split('\n')
        in_function = False
        function_start = 0
        function_name = ""
        
        for i, line in enumerate(lines):
            # Detect function start
            if re.match(r'^\s*\w+\s+\w+\s*\([^)]*\)\s*\{?\s*$', line) or re.match(r'^\s*void\s+\w+\s*\([^)]*\)\s*\{?\s*$', line):
                if '{' in line:
                    in_function = True
                    function_start = i
                    function_name = line.split('(')[0].split()[-1] if '(' in line else "unknown"
            elif '{' in line and not in_function:
                in_function = True
                function_start = i
            elif '}' in line and in_function:
                function_length = i - function_start
                if function_length > 100:
                    self.suggestions.append(f"{filename}: Function '{function_name}' is very long ({function_length} lines), consider refactoring")
                in_function = False
                function_name = ""
    
    def check_includes(self, content, filename):
        """Check include statements"""
        includes = re.findall(r'#include\s+[<"]([^>"]+)[>"]', content)
        if includes:
            print(f"  ✓ Includes {len(includes)} headers")
            
            # Check for ISO includes
            if any('iso' in inc.lower() for inc in includes):
                if 'iso' in filename.lower():
                    print("  ✓ ISO includes found (expected for ISO version)")
                else:
                    self.suggestions.append(f"{filename}: Contains ISO includes but filename doesn't indicate ISO version")
    
    def print_findings(self):
        """Print analysis findings"""
        print("=" * 80)
        print("FINDINGS SUMMARY")
        print("=" * 80)
        
        if self.patterns_found:
            print("\n✓ Good Patterns Found:")
            for pattern, files in self.patterns_found.items():
                print(f"  • {pattern}: {len(files)} file(s)")
                for f in files[:3]:  # Show first 3
                    print(f"    - {f}")
                if len(files) > 3:
                    print(f"    ... and {len(files) - 3} more")
        
        if self.suggestions:
            print(f"\n💡 Suggestions ({len(self.suggestions)}):")
            for suggestion in self.suggestions[:10]:  # Show first 10
                print(f"  • {suggestion}")
            if len(self.suggestions) > 10:
                print(f"  ... and {len(self.suggestions) - 10} more")
        
        if self.issues:
            print(f"\n⚠ Issues ({len(self.issues)}):")
            for issue in self.issues:
                print(f"  • {issue}")
        else:
            print("\n✓ No critical issues found!")
        
        print("\n" + "=" * 80)


def main():
    """Main entry point"""
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        project_root = Path(__file__).parent
    
    analyzer = CodeAnalyzer(project_root)
    analyzer.analyze_code()


if __name__ == '__main__':
    main()

