@echo off
rem Double-click this file to build Burn The Wo and create the download zip in dist\.
rem It runs package-release.ps1 without having to change PowerShell's execution policy.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0package-release.ps1" %*
echo.
pause
