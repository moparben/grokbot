# Grokbot Project Workspace

This repository hosts firmware, documentation, and tooling for the Grokbot robot platform built with ESP-IDF.

## Layout

- `docs/` conceptual and hardware documentation.
- `hardware/` BOM and board-level resources.
- `firmware/` ESP-IDF application sources and board configuration.
- `examples/` sample applications for reference and testing.
- `scripts/` helper utilities for environment setup and flashing.
- `.github/workflows/` CI pipelines for automated builds.

## Getting Started

1. Install the ESP-IDF toolchain that matches your platform. This project targets ESP-IDF v5.5.1.
2. On **Windows (PowerShell)**: run `./scripts/setup_idf_env.ps1 -InstallPython -PreferredPythonVersion 3.13.9` (may require admin) to install or detect Python 3.13 and create a workspace venv, or run `./scripts/create_venv.ps1` if you already have Python 3.13 binary available; then run `& \"$Env:IDF_PATH\\export.ps1\"` or restart PowerShell.
3. On **Linux/macOS**: run `./scripts/setup_env.sh` (this will attempt to clone and run install).
4. Source `scripts/setup_env.sh` or the ESP-IDF `export.sh` script to configure environment variables as described in the ESP-IDF docs.
3. Configure a target board via `idf.py set-target` (defaults defined in `firmware/boards`).
4. Build the main firmware from the `firmware` directory using `idf.py build`.

## Next Steps

- Populate component implementations within `firmware/components/`.
- Flesh out initialization routines in `firmware/main/main.c`.
- Document subsystem designs in `docs/design.md` and hardware notes in `docs/hardware.md`.

## Setup Summary
- Target ESP-IDF version: v5.5.1
- Preferred Python version: 3.13.9

If you want to use the workspace Python virtual environment that the project created, use the Python executable at `grokbot.venv/Scripts/python.exe` (Windows) or `grokbot.venv/bin/python` (POSIX).

Use the VS Code Task 'Setup ESP-IDF (PowerShell)' from `.vscode/tasks.json` or run the scripts manually to get started. There are also helper tasks:

- "IDF: Env Diagnostics (PowerShell)" — runs `env_check.ps1` (checks Python, IDF_PATH, compilers)
- "IDF: Build All (PowerShell)" — runs `build_all.ps1` for the full build flow from a clean environment (may create venv if absent)

If you'd like to use the workspace virtual environment for running IDF scripts and installing requirements, enable it in PowerShell or bash (the setup script optionally creates `grokbot.venv`):
### VS Code Tasks for Build & Flash (Windows PowerShell)

The workspace includes several VS Code tasks in `.vscode/tasks.json` to help run builds and flashing out-of-the-box.

-- "IDF: Build firmware (esp32s3)" — builds the `firmware/` project for ESP32-S3
-- "IDF: Build firmware (esp32p4)" — builds the `firmware/` project for ESP32-P4
-- "IDF: Build example demo-basic (esp32s3/esp32p4)" — builds the sample app in `examples/demo-basic`
-- "IDF: Flash firmware (esp32s3/esp32p4)" — builds & flashes the firmware for the selected target (PowerShell tasks use `scripts/flash_all.ps1`)
-- "IDF: Monitor firmware (esp32s3/esp32p4)" — launch `idf.py monitor` on the correct build folder to see serial logs (PowerShell tasks)

Use the Command Palette (Ctrl+Shift+P) > Run Task to run any of those tasks, or open the Run/Debug sidebar to configure them. On Windows use the "(PowerShell)" tasks; if you need to run POSIX scripts use WSL or run the scripts outside of Windows.


PowerShell:
```
& "${workspaceFolder}\\grokbot.venv\\Scripts\\Activate.ps1"
python --version
```

Bash (POSIX):
```
source ${workspaceFolder}/grokbot.venv/bin/activate
python --version
```
