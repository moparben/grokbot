# Grokbot Environment Setup (ESP-IDF v5.5.1 + Python 3.13.9)

This document explains how to set up the development environment for Grokbot on Windows (PowerShell) and POSIX systems.

## Prerequisites

- Git
- Admin privileges (for Windows winget or install scripts) may be required
- Windows: PowerShell 7+ / `pwsh` recommended

## Windows (PowerShell)

1. Ensure you have a working version of PowerShell (pwsh).
2. Run the PowerShell setup script from the project root (may require elevation):

   pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\setup_idf_env.ps1 -InstallPython -PreferredPythonVersion 3.13.9

3. After the script completes, verify with:

   pwsh -NoProfile -ExecutionPolicy Bypass -Command "python --version; idf.py --version"

4. Run the VS Code 'IDF: Verify environment' task or run the verification script directly:

   - PowerShell: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/verify_env.ps1`
   - Bash: `bash scripts/verify_env.sh`

4. If you need to persist environment variables for ESP-IDF, add this to your PowerShell profile or run the export script created by ESP-IDF:

   & "$Env:IDF_PATH\export.ps1"

## Linux / macOS (Bash)

1. Run the POSIX setup script:

   ./scripts/setup_env.sh

   The script will clone ESP-IDF v5.5.1 (if not already present) and attempt to run its install script.

2. Source the export script to use the `idf.py` tools in the current shell:

   source ./.esp-idf/export.sh

3. Verify:

   python --version
   idf.py --version

## Using the Workspace Python venv

This workspace can use a Python virtual environment at `grokbot.venv` (created by the tooling in this environment). Use the venv python to install python-based tools or run python scripts:

If you ran the setup script with the `-InstallPython -PreferredPythonVersion 3.13.9` flags, `grokbot.venv` will be created using Python 3.13.9 and used by default in this session.

To explicitly set the IDF Python environment to this venv for the current PowerShell session:

```powershell
Set-Item Env:IDF_PYTHON (Resolve-Path .\grokbot.venv\Scripts\python.exe).Path
python --version
idf.py --version
```
## Build & Flash with VS Code Tasks

- Use the Task Runner (Ctrl+Shift+P -> Run Task) to select "IDF: Build firmware (esp32s3)" and "IDF: Flash firmware (esp32s3)".
- The tasks call `idf.py` with SDKCONFIG_DEFAULTS configured to the board specific defaults in `firmware/boards/`.


Example: `grokbot.venv\Scripts\python.exe -m pip install -r ./.esp-idf/requirements.txt`

## Notes

- The `setup_idf_env.ps1` and `setup_env.sh` are scaffolds—edit as needed for your local workflow or to integrate with system package managers.
- Where possible, avoid installing Python system-wide; prefer a project venv.
