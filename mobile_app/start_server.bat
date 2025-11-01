@echo off
REM ========================================
REM myepicEFI API Simulator - Desktop Dashboard
REM ========================================
echo.
echo Starting myepicEFI Desktop Dashboard...
echo.

REM Get the directory where this batch file is located
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python is not installed or not in PATH
    echo Please install Python 3.7+ and try again
    pause
    exit /b 1
)

REM Check if api_simulator.py exists
if not exist "python\api_simulator.py" (
    echo [ERROR] api_simulator.py not found in python directory
    echo Current directory: %CD%
    pause
    exit /b 1
)

REM Check if port 8080 is already in use (server already running)
netstat -ano | findstr ":8080" >nul 2>&1
if not errorlevel 1 (
    echo [INFO] Server is already running on port 8080
    echo.
    echo Opening dashboard in browser...
    timeout /t 1 /nobreak >nul
    
    REM Test if server responds before opening browser
    powershell -Command "try { $response = Invoke-WebRequest -Uri 'http://localhost:8080/health' -TimeoutSec 2 -UseBasicParsing -ErrorAction Stop; exit 0 } catch { exit 1 }" >nul 2>&1
    if errorlevel 1 (
        echo [WARNING] Server on port 8080 not responding
        echo.
        choice /C YN /M "Kill existing process and start new server"
        if errorlevel 2 goto :open_browser_anyway
        if errorlevel 1 (
            echo.
            echo Finding process on port 8080...
            for /f "tokens=5" %%a in ('netstat -ano ^| findstr ":8080" ^| findstr "LISTENING"') do (
                echo Killing process %%a...
                taskkill /PID %%a /F >nul 2>&1
            )
            timeout /t 2 /nobreak >nul
            goto :start_server
        )
    )
    
    :open_browser_anyway
    REM Server is running and responding - just open browser
    start "" "http://localhost:8080"
    echo.
    echo Dashboard opened in browser!
    echo Server is already running in another window.
    echo.
    echo To stop the server: Close the server window or press Ctrl+C in that window
    echo.
    pause
    exit /b 0
)

:start_server
REM Change to python directory
cd /d "%SCRIPT_DIR%python"

REM Display startup info
echo.
echo ========================================
echo myepicEFI Desktop Dashboard Server
echo ========================================
echo.
echo Starting server on port 8080...
echo.

REM Start the Python server (will run in foreground)
start "myepicEFI Server" python api_simulator.py

REM Wait for server to start, then open browser
echo Waiting for server to initialize...
timeout /t 3 /nobreak >nul

REM Open browser to dashboard
start "" "http://localhost:8080"

echo.
echo Browser opened to: http://localhost:8080
echo.
echo The dashboard is now available in your browser.
echo The server window is running separately.
echo.
echo To stop: Close the server window or press Ctrl+C in that window
echo ========================================
echo.
echo Press any key to exit this window (server will continue running)...
pause >nul
