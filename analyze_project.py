#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
USB_HID_CAN_BRIDGE Project Analyzer
Comprehensive analysis of project structure, code quality, and configuration.
"""

import os
import sys
import json
import re
from pathlib import Path
from datetime import datetime
from collections import defaultdict

# Fix Windows console encoding
if sys.platform == 'win32':
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', errors='replace')

class ProjectAnalyzer:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.issues = []
        self.warnings = []
        self.info = []
        self.stats = defaultdict(int)
        
    def analyze(self):
        """Run all analysis functions"""
        print("=" * 80)
        print("USB_HID_CAN_BRIDGE Project Analysis")
        print("=" * 80)
        print(f"Project Root: {self.project_root}")
        print(f"Analysis Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        
        # Run all checks
        self.check_project_structure()
        self.check_memory_bank()
        self.check_firmware_files()
        self.check_dependencies()
        self.check_code_quality()
        self.check_configuration()
        self.analyze_statistics()
        
        # Print results
        self.print_summary()
        
    def check_project_structure(self):
        """Check project directory structure"""
        print("[1] Checking Project Structure...")
        
        expected_dirs = ['.project', 'epic_can_logger', 'pics']
        for dir_name in expected_dirs:
            dir_path = self.project_root / dir_name
            if dir_path.exists():
                self.info.append(f"✓ Found directory: {dir_name}")
                self.stats['directories'] += 1
            else:
                self.warnings.append(f"⚠ Missing directory: {dir_name}")
        
        # Check for main firmware files
        firmware_dir = self.project_root / 'epic_can_logger'
        if firmware_dir.exists():
            main_files = ['epic_can_logger.ino', 'epic_can_logger_iso.ino']
            for fname in main_files:
                fpath = firmware_dir / fname
                if fpath.exists():
                    size = fpath.stat().st_size
                    self.info.append(f"✓ Found firmware: {fname} ({size:,} bytes)")
                    self.stats['firmware_files'] += 1
                else:
                    self.warnings.append(f"⚠ Missing firmware: {fname}")
        
        print("   ✓ Project structure check complete\n")
    
    def check_memory_bank(self):
        """Check memory bank files"""
        print("[2] Checking Memory Bank Files...")
        
        mb_dir = self.project_root / '.project'
        required_files = [
            'projectbrief.md',
            'productContext.md',
            'activeContext.md',
            'systemPatterns.md',
            'techContext.md',
            'progress.md'
        ]
        
        for fname in required_files:
            fpath = mb_dir / fname
            if fpath.exists():
                size = fpath.stat().st_size
                lines = len(fpath.read_text(encoding='utf-8', errors='ignore').split('\n'))
                self.info.append(f"✓ Memory bank: {fname} ({lines} lines, {size:,} bytes)")
                self.stats['memory_bank_files'] += 1
                
                # Check if file has content
                if size < 100:
                    self.warnings.append(f"⚠ Memory bank file {fname} is very small ({size} bytes)")
            else:
                self.issues.append(f"✗ Missing memory bank file: {fname}")
        
        print(f"   ✓ Memory bank check complete ({self.stats['memory_bank_files']}/{len(required_files)} files)\n")
    
    def check_firmware_files(self):
        """Analyze firmware .ino files"""
        print("[3] Analyzing Firmware Files...")
        
        firmware_dir = self.project_root / 'epic_can_logger'
        if not firmware_dir.exists():
            self.warnings.append("⚠ Firmware directory not found")
            return
        
        ino_files = list(firmware_dir.glob('*.ino'))
        for ino_file in ino_files:
            self.analyze_ino_file(ino_file)
        
        print(f"   ✓ Firmware analysis complete ({len(ino_files)} files)\n")
    
    def analyze_ino_file(self, file_path):
        """Analyze a single .ino file"""
        try:
            content = file_path.read_text(encoding='utf-8', errors='ignore')
            lines = content.split('\n')
            self.stats['total_code_lines'] += len(lines)
            
            # Count includes
            includes = [l for l in lines if l.strip().startswith('#include')]
            self.stats['includes'] += len(includes)
            
            # Check for critical functions
            has_setup = 'void setup()' in content or 'void setup(void)' in content
            has_loop = 'void loop()' in content or 'void loop(void)' in content
            
            if has_setup and has_loop:
                self.info.append(f"✓ {file_path.name}: Has setup() and loop() ({len(lines)} lines)")
            else:
                self.issues.append(f"✗ {file_path.name}: Missing setup() or loop()")
            
            # Check for ISO-specific code
            if 'iso15765' in content.lower() or 'uds' in content.lower():
                if 'iso' in file_path.name.lower():
                    self.info.append(f"✓ {file_path.name}: Contains ISO 15765/14229 code (expected)")
                else:
                    self.warnings.append(f"⚠ {file_path.name}: Contains ISO code but filename doesn't indicate ISO version")
            
            # Check for debug macros
            if 'DEBUG_' in content:
                self.info.append(f"✓ {file_path.name}: Uses debug macros")
            
            # Check for error handling
            if 'error' in content.lower() or 'Error' in content:
                self.info.append(f"✓ {file_path.name}: Contains error handling")
            
        except Exception as e:
            self.issues.append(f"✗ Error analyzing {file_path.name}: {e}")
    
    def check_dependencies(self):
        """Check for dependency files and documentation"""
        print("[4] Checking Dependencies...")
        
        firmware_dir = self.project_root / 'epic_can_logger'
        if firmware_dir.exists():
            # Check for library includes
            dep_docs = ['DEPENDENCIES.md', 'README.md']
            for doc in dep_docs:
                doc_path = firmware_dir / doc
                if doc_path.exists():
                    self.info.append(f"✓ Found dependency doc: {doc}")
                else:
                    self.warnings.append(f"⚠ Missing dependency doc: {doc}")
            
            # Check for library ZIP files in parent
            zip_files = list(self.project_root.glob('*.zip'))
            if zip_files:
                for zf in zip_files:
                    self.info.append(f"✓ Found library ZIP: {zf.name}")
            else:
                self.warnings.append("⚠ No library ZIP files found in project root")
        
        print("   ✓ Dependencies check complete\n")
    
    def check_code_quality(self):
        """Check code quality indicators"""
        print("[5] Checking Code Quality...")
        
        firmware_dir = self.project_root / 'epic_can_logger'
        if not firmware_dir.exists():
            return
        
        # Check for .cpp and .h files
        cpp_files = list(firmware_dir.glob('*.cpp'))
        h_files = list(firmware_dir.glob('*.h'))
        
        self.stats['cpp_files'] = len(cpp_files)
        self.stats['header_files'] = len(h_files)
        
        # Check for proper module structure
        expected_modules = ['sd_logger', 'rusefi_dbc', 'config_manager']
        if 'epic_can_logger_iso.ino' in [f.name for f in firmware_dir.glob('*.ino')]:
            expected_modules.extend(['iso15765', 'uds'])
        
        for module in expected_modules:
            has_cpp = any(f.name.startswith(module) and f.suffix == '.cpp' for f in cpp_files)
            has_h = any(f.name.startswith(module) and f.suffix == '.h' for f in h_files)
            
            if has_cpp and has_h:
                self.info.append(f"✓ Module structure: {module}.h/.cpp")
            elif has_h:
                self.warnings.append(f"⚠ Module {module} has header but no implementation")
            elif has_cpp:
                self.warnings.append(f"⚠ Module {module} has implementation but no header")
        
        # Check for documentation
        md_files = list(firmware_dir.glob('*.md'))
        self.stats['documentation_files'] = len(md_files)
        
        if len(md_files) > 10:
            self.info.append(f"✓ Comprehensive documentation ({len(md_files)} markdown files)")
        
        print("   ✓ Code quality check complete\n")
    
    def check_configuration(self):
        """Check configuration files"""
        print("[6] Checking Configuration...")
        
        # Check MCP configuration
        mcp_config = self.project_root / '.cursor' / 'mcp.json'
        if mcp_config.exists():
            try:
                with open(mcp_config, 'r') as f:
                    mcp_data = json.load(f)
                self.info.append("✓ MCP configuration file exists and is valid JSON")
                
                # Check if it uses python command
                if 'mcpServers' in mcp_data:
                    for server_name, server_config in mcp_data['mcpServers'].items():
                        if server_config.get('command') == 'python':
                            self.info.append(f"✓ MCP server '{server_name}' uses python command (fixed)")
                        elif server_config.get('command') == 'uvx':
                            self.warnings.append(f"⚠ MCP server '{server_name}' still uses uvx (may not work)")
            except json.JSONDecodeError:
                self.issues.append("✗ MCP configuration file has invalid JSON")
        else:
            self.warnings.append("⚠ MCP configuration file not found")
        
        print("   ✓ Configuration check complete\n")
    
    def analyze_statistics(self):
        """Calculate and display statistics"""
        print("[7] Calculating Statistics...")
        
        firmware_dir = self.project_root / 'epic_can_logger'
        if firmware_dir.exists():
            # Count all code files
            all_code_files = (
                list(firmware_dir.glob('*.ino')) +
                list(firmware_dir.glob('*.cpp')) +
                list(firmware_dir.glob('*.h'))
            )
            
            total_size = sum(f.stat().st_size for f in all_code_files)
            self.stats['total_code_size'] = total_size
            
            # Count documentation
            all_docs = list(firmware_dir.glob('*.md')) + list(self.project_root.glob('*.md'))
            self.stats['total_documentation'] = len(all_docs)
        
        print("   ✓ Statistics calculated\n")
    
    def print_summary(self):
        """Print analysis summary"""
        print("\n" + "=" * 80)
        print("ANALYSIS SUMMARY")
        print("=" * 80)
        
        print(f"\n📊 Statistics:")
        print(f"   • Memory Bank Files: {self.stats['memory_bank_files']}/6")
        print(f"   • Firmware Files: {self.stats['firmware_files']}")
        print(f"   • C++ Modules: {self.stats['cpp_files']}")
        print(f"   • Header Files: {self.stats['header_files']}")
        print(f"   • Documentation Files: {self.stats['documentation_files']}")
        print(f"   • Total Code Lines: {self.stats['total_code_lines']:,}")
        print(f"   • Total Code Size: {self.stats['total_code_size']:,} bytes ({self.stats['total_code_size']/1024:.1f} KB)")
        
        if self.info:
            print(f"\n✓ Info Messages ({len(self.info)}):")
            for msg in self.info[:10]:  # Show first 10
                print(f"   {msg}")
            if len(self.info) > 10:
                print(f"   ... and {len(self.info) - 10} more")
        
        if self.warnings:
            print(f"\n⚠ Warnings ({len(self.warnings)}):")
            for msg in self.warnings:
                print(f"   {msg}")
        
        if self.issues:
            print(f"\n✗ Issues ({len(self.issues)}):")
            for msg in self.issues:
                print(f"   {msg}")
        
        # Overall status
        print("\n" + "=" * 80)
        if self.issues:
            status = "⚠ HAS ISSUES"
            color = "⚠"
        elif self.warnings:
            status = "✓ GOOD (with warnings)"
            color = "✓"
        else:
            status = "✓ EXCELLENT"
            color = "✓"
        
        print(f"{color} Overall Status: {status}")
        print("=" * 80)
        
        # Recommendations
        if self.issues or self.warnings:
            print("\n💡 Recommendations:")
            if self.issues:
                print("   1. Fix all issues before deployment")
            if self.warnings:
                print("   2. Review warnings and address if needed")
            print("   3. Run tests to verify functionality")
            print("   4. Review memory bank files for completeness")


def main():
    """Main entry point"""
    # Get project root (current directory or provided as argument)
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        # Assume script is in project root, or adjust as needed
        project_root = Path(__file__).parent
    
    analyzer = ProjectAnalyzer(project_root)
    analyzer.analyze()


if __name__ == '__main__':
    main()

