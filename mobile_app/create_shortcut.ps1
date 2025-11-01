# Create myepicEFI Desktop Shortcut
# Matches dashboard UI styling and colors

Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Creating myepicEFI Desktop Shortcut" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

# Get paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$PythonDir = Join-Path $ScriptDir "python"
$IconScript = Join-Path $PythonDir "create_desktop_icon.py"
$BatFile = Join-Path $ScriptDir "start_server.bat"
$DesktopPath = [Environment]::GetFolderPath("Desktop")
$ShortcutPath = Join-Path $DesktopPath "myepicEFI.lnk"

# Check if PIL is available for icon creation
Write-Host "Creating icon..." -ForegroundColor Yellow
$hasPIL = $false

$testCmd = 'from PIL import Image, ImageDraw, ImageFont'
$result = python -c $testCmd 2>&1
if ($LASTEXITCODE -eq 0) {
    $hasPIL = $true
}

if (-not $hasPIL) {
    Write-Host "  PIL (Pillow) not found. Installing..." -ForegroundColor Yellow
    pip install Pillow --quiet 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        $hasPIL = $true
        Write-Host "  ✓ Pillow installed" -ForegroundColor Green
    } else {
        Write-Host "  ⚠ Could not install Pillow. Creating shortcut with default icon." -ForegroundColor Yellow
        $hasPIL = $false
    }
}

# Create icon if PIL is available
$IconPath = Join-Path $ScriptDir "myepicEFI.ico"
if ($hasPIL) {
    try {
        Set-Location $ScriptDir
        python $IconScript
        if (Test-Path $IconPath) {
            Write-Host "  ✓ Icon created" -ForegroundColor Green
        } else {
            Write-Host "  ⚠ Icon creation may have failed, using default" -ForegroundColor Yellow
            $IconPath = $null
        }
    } catch {
        Write-Host "  ⚠ Could not create custom icon" -ForegroundColor Yellow
        $IconPath = $null
    }
}

# Create shortcut
Write-Host "`nCreating desktop shortcut..." -ForegroundColor Yellow

try {
    $WshShell = New-Object -ComObject WScript.Shell
    $Shortcut = $WshShell.CreateShortcut($ShortcutPath)
    $Shortcut.TargetPath = $BatFile
    $Shortcut.WorkingDirectory = $ScriptDir
    $Shortcut.Description = "myepicEFI Desktop Dashboard - Real-time ECU monitoring and tuning"
    
    # Set icon if available
    if ($IconPath -and (Test-Path $IconPath)) {
        $Shortcut.IconLocation = "$IconPath,0"
        Write-Host "  ✓ Custom icon applied" -ForegroundColor Green
    } else {
        # Use batch file icon as fallback
        $Shortcut.IconLocation = "$env:SystemRoot\System32\shell32.dll,137"
        Write-Host "  ✓ Default icon applied" -ForegroundColor Yellow
    }
    
    $Shortcut.Save()
    
    Write-Host "`n✓ Desktop shortcut created successfully!" -ForegroundColor Green
    Write-Host "  Location: $ShortcutPath" -ForegroundColor Cyan
    Write-Host "`nYou can now double-click 'myepicEFI' on your desktop to launch the dashboard!`n" -ForegroundColor Yellow
    
} catch {
    Write-Host "`n✗ Error creating shortcut: $_" -ForegroundColor Red
    exit 1
}

# Optionally open desktop to show the shortcut
$show = Read-Host "`nShow desktop shortcut? (Y/N)"
if ($show -eq "Y" -or $show -eq "y") {
    $argList = @('/select', $ShortcutPath)
    Start-Process "explorer.exe" -ArgumentList $argList
}
