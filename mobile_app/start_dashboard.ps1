# myepicEFI Desktop Dashboard Startup Script
# PowerShell version for better error handling and cross-platform compatibility

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "myepicEFI Desktop Dashboard Server" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Get script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PythonDir = Join-Path $ScriptDir "python"
$SimulatorPath = Join-Path $PythonDir "api_simulator.py"

# Check if Python is installed
try {
    $pythonVersion = python --version 2>&1
    if ($LASTEXITCODE -ne 0) { throw "Python not found" }
    Write-Host "[OK] Python found: $pythonVersion" -ForegroundColor Green
} catch {
    Write-Host "[ERROR] Python is not installed or not in PATH" -ForegroundColor Red
    Write-Host "Please install Python 3.7+ and add it to your PATH" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

# Check if simulator script exists
if (-not (Test-Path $SimulatorPath)) {
    Write-Host "[ERROR] api_simulator.py not found at: $SimulatorPath" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Check if port 8080 is in use
$portInUse = Get-NetTCPConnection -LocalPort 8080 -ErrorAction SilentlyContinue | Where-Object { $_.State -eq "Listen" }

if ($portInUse) {
    Write-Host "[WARNING] Port 8080 is already in use" -ForegroundColor Yellow
    Write-Host ""
    $response = Read-Host "Kill existing process? (Y/N)"
    
    if ($response -eq "Y" -or $response -eq "y") {
        Write-Host "Finding and killing process on port 8080..." -ForegroundColor Yellow
        
        $connections = Get-NetTCPConnection -LocalPort 8080 -ErrorAction SilentlyContinue | Where-Object { $_.State -eq "Listen" }
        foreach ($conn in $connections) {
            $process = Get-Process -Id $conn.OwningProcess -ErrorAction SilentlyContinue
            if ($process) {
                Write-Host "  Killing process: $($process.Name) (PID: $($process.Id))" -ForegroundColor Yellow
                Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
            }
        }
        
        Start-Sleep -Seconds 2
        Write-Host "[OK] Port 8080 is now free" -ForegroundColor Green
    } else {
        Write-Host "[INFO] Using existing server. Opening dashboard..." -ForegroundColor Cyan
        Start-Process "http://localhost:8080"
        exit 0
    }
}

# Change to python directory
Set-Location $PythonDir

# Display startup info
Write-Host ""
Write-Host "Starting server..." -ForegroundColor Green
Write-Host ""
Write-Host "Dashboard URL: http://localhost:8080" -ForegroundColor Cyan
Write-Host ""
Write-Host "Press Ctrl+C to stop the server" -ForegroundColor Yellow
Write-Host "========================================`n" -ForegroundColor Cyan

# Start the Python server
try {
    python api_simulator.py
} catch {
    Write-Host ""
    Write-Host "[ERROR] Failed to start server: $_" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "Server stopped." -ForegroundColor Yellow
Read-Host "Press Enter to exit"

