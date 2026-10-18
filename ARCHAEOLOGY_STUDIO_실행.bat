@echo off
chcp 65001 >nul
setlocal

set "PLUGIN_DIR=%~dp0"
if "%PLUGIN_DIR:~-1%"=="\" set "PLUGIN_DIR=%PLUGIN_DIR:~0,-1%"

echo [ARCHAEO STUDIO 5.0] 브리지 서버 및 고고학 고찰·논문 스튜디오를 시작합니다...
node "%PLUGIN_DIR%\scripts\gemini_mcp_server.js" --standalone
start "" "%PLUGIN_DIR%\bin\wordcraft\wordcraft-0.7.0-windows-x64-portable\wordcraft.exe"
