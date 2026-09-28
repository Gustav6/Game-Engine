@echo off
.\Tools\Premake\Premake5.exe vs2026

if %ERRORLEVEL% neq 0 (
    echo Premake failed!
    pause
    exit /b %ERRORLEVEL%
)