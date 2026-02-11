@echo off
REM ESP-IDF Environment Setup and Build Script for Voice Assistant

echo Setting up ESP-IDF environment...

REM Clear any corrupted environment variables
set IDF_PATH=

REM Set correct paths
set IDF_PATH=d:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1
set IDF_PYTHON=d:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env\Scripts\python.exe
set PATH=d:\esp32_projects\Espressif\tools\cmake\3.30.2\bin;d:\esp32_projects\Espressif\tools\ninja\1.12.1;d:\esp32_projects\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin;d:\esp32_projects\Espressif\tools\riscv32-esp-elf\esp-14.2.0_20241119\riscv32-esp-elf\bin;%PATH%

echo Environment variables set:
echo IDF_PATH=%IDF_PATH%
echo IDF_PYTHON=%IDF_PYTHON%

REM Change to the voice assistant directory
cd /d d:\esp32_projects\grokbot\examples\voice-assistant-s3

echo.
echo Testing idf.py...
"%IDF_PYTHON%" "%IDF_PATH%\tools\idf.py" --version

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: idf.py test failed!
    pause
    exit /b 1
)

echo.
echo Setting target to ESP32-S3...
"%IDF_PYTHON%" "%IDF_PATH%\tools\idf.py" set-target esp32s3

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: set-target failed!
    pause
    exit /b 1
)

echo.
echo Building voice assistant...
"%IDF_PYTHON%" "%IDF_PATH%\tools\idf.py" -B build-esp32s3 -DSDKCONFIG_DEFAULTS=../../firmware/boards/esp32s3/sdkconfig.defaults build

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo To flash the device, run:
echo "%IDF_PYTHON%" "%IDF_PATH%\tools\idf.py" -B build-esp32s3 flash
echo.
echo To monitor the device, run:
echo "%IDF_PYTHON%" "%IDF_PATH%\tools\idf.py" -B build-esp32s3 monitor
echo.
pause