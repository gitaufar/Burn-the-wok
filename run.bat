@echo off
rem Shortcut for tools\dev.ps1, like "npm run": .\run, .\run release, .\run build, .\run help ...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\dev.ps1" %*
exit /b %ERRORLEVEL%
