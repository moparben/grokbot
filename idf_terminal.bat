@echo off
REM Quick launcher for ESP-IDF PowerShell environment
REM Double-click this file to open a properly configured terminal

cd /d "%~dp0"
pwsh -NoExit -ExecutionPolicy Bypass -Command ". .\scripts\activate_grokbot.ps1"
