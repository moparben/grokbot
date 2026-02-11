# Grokbot Troubleshooting (Windows PowerShell focused)

This document provides a checklist and common errors when building or flashing Grokbot firmware using ESP-IDF on Windows.

## Common error: idf.py exit code 1 when building
- Cause: `IDF_PATH` is not set or `idf.py` is not available in PATH.
  - Solution: Source the `export.ps1` script from your IDF installation directory or run `scripts\setup_idf_env.ps1` to install and configure ESP-IDF.
    ```powershell
    & $Env:IDF_PATH\export.ps1
    python --version
    idf.py --version
    ```
  - Solution: Use the workspace venv at `grokbot.venv` or install Python 3.13.9 and make sure it is on PATH.
  - Solution: Use the workspace venv at `grokbot.venv` (created by `setup_idf_env.ps1`) or install Python 3.13.9 and make sure it is on PATH. To use the workspace venv in PowerShell:
    ```powershell
    & .\grokbot.venv\Scripts\Activate.ps1
    python --version
    idf.py --version
    ```
  - Alternative: If you have the `py` Python launcher, you can use `py -3.13` to create/activate the venv:
    ```powershell
    py -3.13 -m venv .\grokbot.venv
    & .\grokbot.venv\Scripts\Activate.ps1
    python --version
    ### Installing Python 3.13 on Windows (winget)

    If you prefer installing Python 3.13 system-wide, you can use winget (needs admin):

    ```powershell
    winget install --id=Python.Python.3.13 -e --accept-package-agreements --accept-source-agreements
    ```

    After installation, open a new PowerShell session and verify `python --version` shows `3.13.x`.
    ```
  - Solution: Clean and build again, inspect the build error messages. Example:
    ```powershell
    idf.py -B build-esp32s3 fullclean
    idf.py -B build-esp32s3 build
    ```
- Cause: SDKCONFIG defaults not found or incorrect.
  - Solution: Make sure the correct board defaults are used: `-DSDKCONFIG_DEFAULTS=boards/esp32s3/sdkconfig.defaults` and that the default files exist.

## Verifying environment
- Run the verify script:
  ```powershell
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\verify_env.ps1
  ```

## Helpful debug commands
- Confirm where `idf.py` exists:
  ```powershell
  Get-Command idf.py
  ```
- Show active `IDF_PATH`:
  ```powershell
  echo $Env:IDF_PATH
  ```
- Show Python location:
  ```powershell
  python -c "import sys; print(sys.executable)"
  ```

## Capturing build logs for investigation
- Use the debug build script to capture a full build log for the given target:
  ```powershell
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\debug_build.ps1 -Target esp32s3
  ```
  The script writes a timestamped log to the `firmware\logs` directory that you can review and attach for diagnosis.

## If all else fails
1. Run `scripts\setup_idf_env.ps1 -InstallPython` and follow prompts.
2. Re-open PowerShell and source `export.ps1`.
3. Run the build again, capturing full logs.

If you collect log output and paste it here, I can help interpret the errors and suggest fixes. Include the sections around the first ERROR or E: messages to make diagnosis faster.