@echo off
set IDF_PATH=D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1
set PATH=D:\esp32_projects\Espressif\tools\cmake\3.30.2\bin;D:\esp32_projects\Espressif\tools\ninja\1.12.1;D:\esp32_projects\Espressif\tools\riscv32-esp-elf\esp-14.2.0_20241119\riscv32-esp-elf\bin;C:\Program Files\Git\cmd;C:\Windows\System32;C:\Windows
cd /d D:\esp32_projects\grokbot\firmware
if exist build rmdir /s /q build
D:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env\Scripts\python.exe D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1\tools\idf.py set-target esp32p4 build
