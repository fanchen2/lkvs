#!/usr/bin/env python3
"""
Quick syntax check for boot_repeat.py
"""
import ast
import sys

try:
    with open('boot_repeat.py', 'r') as f:
        code = f.read()
    ast.parse(code)
    print("✓ boot_repeat.py syntax is valid")
    sys.exit(0)
except SyntaxError as e:
    print(f"✗ Syntax error in boot_repeat.py: {e}")
    sys.exit(1)
