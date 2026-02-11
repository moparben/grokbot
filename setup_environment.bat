@echo off
REM ESP-IDF Environment Setup for Grokbot

set IDF_TOOLS_PATH=D:\esp32_projects\Espressif
set IDF_PATH=D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1
set IDF_PYTHON=D:\esp32_projects\grokbot\grokbot.venv\Scripts\python.exe
set IDF_PYTHON_ENV_PATH=

REM Add tools to PATH
set PATH=%IDF_TOOLS_PATH%\tools\cmake\3.30.2\bin;%IDF_TOOLS_PATH%\tools\ninja\1.12.1;%IDF_TOOLS_PATH%\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin;%IDF_TOOLS_PATH%\tools\riscv32-esp-elf\esp-14.2.0_20241119\riscv32-esp-elf\bin;%PATH%

echo ESP-IDF Environment Ready
echo IDF_PATH: %IDF_PATH%
echo IDF_PYTHON: %IDF_PYTHON%
echo.

REM Change to voice-assistant-s3 directory
cd /d D:\esp32_projects\grokbot\examples\voice-assistant-s3

echo Current directory: %CD%
echo.

cmd /k