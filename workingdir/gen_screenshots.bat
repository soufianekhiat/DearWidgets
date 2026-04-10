@echo off
setlocal

:: Output directory — relative to workingdir, can be overridden via first argument
if "%~1"=="" (
    set OUT=%~dp0..\docs\screenshots
) else (
    set OUT=%~1
)

:: Normalize to an absolute path
for %%I in ("%OUT%") do set OUT=%%~fI

echo Generating screenshots into: %OUT%
mkdir "%OUT%" 2>nul

:: Run the demo in screenshot mode.
:: The demo renders a fixed number of warmup frames, captures, then exits cleanly.
dearwidgetsdemo.exe --screenshot "%OUT%"

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Demo exited with code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo.
echo Screenshots written:
dir /b "%OUT%\*.png" 2>nul

endlocal
