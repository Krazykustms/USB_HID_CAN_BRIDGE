@echo off
REM Run Virtual System Tests
REM Tests boot patterns, connections, and functions

echo ========================================
echo myepicEFI Virtual System Test Suite
echo ========================================
echo.

cd /d "%~dp0"
python test_virtual_system.py

echo.
pause

