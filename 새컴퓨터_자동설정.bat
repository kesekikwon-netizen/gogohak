@echo off
chcp 65001 >nul
setlocal EnableDelayedExpansion

echo ============================================================================
echo   [ARCHAEO STUDIO 5.0] 새 컴퓨터 1클릭 자동 설치 및 환경 구성
echo   (Gemini '작업' 모드 x DBpia AI x WordCraft 네이티브 고찰·논문 워크스테이션)
echo ============================================================================
echo.

set "SRC_DIR=%~dp0"
if "%SRC_DIR:~-1%"=="\" set "SRC_DIR=%SRC_DIR:~0,-1%"

set "PLUGIN_DST=%USERPROFILE%\.gemini\config\plugins\gemini-desktop"
set "SKILL_DST_1=%USERPROFILE%\.gemini\config\skills\archaeo-studio-pro"
set "SKILL_DST_2=%USERPROFILE%\.gemini\config\skills\rea-gemini-app-bridge"
set "MCP_CFG=%USERPROFILE%\.gemini\antigravity\mcp_config.json"
set "REPORTS_DIR=%USERPROFILE%\archaeology_reports"

echo [1/5] 고고학 고찰·논문 출력 폴더 생성 중: %REPORTS_DIR%
if not exist "%REPORTS_DIR%" mkdir "%REPORTS_DIR%"

echo [2/5] ARCHAEO STUDIO 플러그인 및 WordCraft/PdfCraft 포터블 복사 중...
if not exist "%PLUGIN_DST%" mkdir "%PLUGIN_DST%"
if /I not "%SRC_DIR%"=="%PLUGIN_DST%" (
    xcopy "%SRC_DIR%\*" "%PLUGIN_DST%\" /E /I /Y /Q >nul
)

echo [3/5] 안티그라비티 스킬(archaeo-studio-pro, rea-gemini-app-bridge) 등록 중...
if exist "%SRC_DIR%\skills_backup\archaeo-studio-pro" (
    if not exist "%SKILL_DST_1%" mkdir "%SKILL_DST_1%"
    xcopy "%SRC_DIR%\skills_backup\archaeo-studio-pro\*" "%SKILL_DST_1%\" /E /I /Y /Q >nul
)
if exist "%SRC_DIR%\skills_backup\rea-gemini-app-bridge" (
    if not exist "%SKILL_DST_2%" mkdir "%SKILL_DST_2%"
    xcopy "%SRC_DIR%\skills_backup\rea-gemini-app-bridge\*" "%SKILL_DST_2%\" /E /I /Y /Q >nul
)

echo [4/5] Antigravity MCP 설정(mcp_config.json)에 gemini-desktop 자동 등록 중...
node -e "const fs=require('fs'),path=require('path');const p=process.env.MCP_CFG;const dir=path.dirname(p);if(!fs.existsSync(dir))fs.mkdirSync(dir,{recursive:true});let cfg={mcpServers:{}};if(fs.existsSync(p)){try{cfg=JSON.parse(fs.readFileSync(p,'utf8'));}catch(e){}}if(!cfg.mcpServers)cfg.mcpServers={};const srvPath=path.join(process.env.PLUGIN_DST,'scripts','gemini_mcp_server.js');cfg.mcpServers['gemini-desktop']={command:'node',args:[srvPath],env:{GEMINI_CDP_PORT:'9223',GEMINI_BRIDGE_PORT:'18787'}};fs.writeFileSync(p,JSON.stringify(cfg,null,2),'utf8');console.log('  -> MCP 등록 완료:',p);"

echo [5/5] 바탕화면에 [ARCHAEO STUDIO 실행.bat] 생성 중...
set "DESKTOP_BAT=%USERPROFILE%\Desktop\ARCHAEO_STUDIO_실행.bat"
(
echo @echo off
echo chcp 65001 ^>nul
echo call "%%USERPROFILE%%\.gemini\config\plugins\gemini-desktop\ARCHAEOLOGY_STUDIO_실행.bat"
) > "%DESKTOP_BAT%"

echo.
echo ============================================================================
echo   설치가 모두 완료되었습니다!
echo   1. Antigravity 앱을 실행하고 우측 하단 [Gemini] 패널에서 구글 로그인을 확인하세요.
echo   2. 바탕화면의 [ARCHAEO_STUDIO_실행.bat] 또는
echo      %PLUGIN_DST%\bin\wordcraft\wordcraft-0.7.0-windows-x64-portable\wordcraft.exe
echo      를 실행하면 즉시 고고학 고찰·논문 스튜디오가 작동합니다.
echo ============================================================================
pause
