@echo off
REM ESP32-P4 Build Script for Grokbot
REM Run this from Windows Explorer (double-click) or from a fresh CMD prompt
REM This avoids any VS Code environment issues

echo Setting up ESP-IDF environment...
set IDF_PATH=D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1
set IDF_TOOLS_PATH=D:\esp32_projects\Espressif
set TEMP=C:\Users\%USERNAME%\AppData\Local\Temp
set TMP=C:\Users\%USERNAME%\AppData\Local\Temp

REM Set minimal PATH with only required tools
set PATH=D:\esp32_projects\Espressif\tools\cmake\3.30.2\bin;D:\esp32_projects\Espressif\tools\ninja\1.12.1;D:\esp32_projects\Espressif\tools\riscv32-esp-elf\esp-14.2.0_20241119\riscv32-esp-elf\bin;C:\Program Files\Git\cmd;C:\Windows\System32;C:\Windows

echo Cleaning build directory...
cd /d D:\esp32_projects\grokbot\firmware
if exist build rmdir /s /q build

echo Building for ESP32-P4...
D:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env\Scripts\python.exe %IDF_PATH%\tools\idf.py set-target esp32p4

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo *** BUILD FAILED ***
    echo Exit code: %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)

D:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env\Scripts\python.exe %IDF_PATH%\tools\idf.py build

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo *** BUILD FAILED ***
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo *** BUILD SUCCESSFUL ***
pause
