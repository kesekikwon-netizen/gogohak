# ARCHAEO STUDIO PRO 4.0 - Portable Setup & Verification Script
# Run this script on any Windows PC to auto-configure mcp.json for the current user profile and start the bridge daemon.

$ErrorActionPreference = "Stop"
$UserHome = $env:USERPROFILE.Replace('\', '/')
$PluginDir = "$UserHome/.gemini/config/plugins/gemini-desktop"
$ScriptPath = "$PluginDir/scripts/gemini_mcp_server.js"
$McpJsonPath = "$PluginDir/mcp.json"

Write-Host "[ARCHAEO STUDIO PRO 4.0] Configuring portable environment..." -ForegroundColor Cyan

if (-not (Test-Path $ScriptPath)) {
    Write-Error "Cannot find gemini_mcp_server.js at: $ScriptPath"
    exit 1
}

$McpConfig = @{
    mcpServers = @{
        "gemini-desktop" = @{
            command = "node"
            args    = @($ScriptPath)
            env     = @{}
        }
    }
} | ConvertTo-Json -Depth 5

[System.IO.File]::WriteAllText($McpJsonPath, $McpConfig, (New-Object System.Text.UTF8Encoding($false)))
Write-Host "[OK] Updated mcp.json path: $McpJsonPath" -ForegroundColor Green

# Stop existing daemon instances and start fresh v4.0 daemon outside job object via WMI
Get-CimInstance Win32_Process | Where-Object { $_.Name -eq 'node.exe' -and $_.CommandLine -like '*gemini_mcp_server.js*--daemon*' } | ForEach-Object {
    Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue
}
Start-Sleep -Milliseconds 500

$NodeExe = (Get-Command node).Source
$StartupInfo = New-CimInstance -ClassName Win32_ProcessStartup -ClientOnly -Property @{ ShowWindow = [uint16]0 }
$null = Invoke-CimMethod -ClassName Win32_Process -MethodName Create -Arguments @{
    CommandLine = "`"$NodeExe`" `"$ScriptPath`" --daemon"
    ProcessStartupInformation = $StartupInfo
}
Start-Sleep -Seconds 2

try {
    $status = Invoke-RestMethod -Uri "http://127.0.0.1:18787/pipeline-status" -TimeoutSec 5
    Write-Host "[OK] ARCHAEO STUDIO PRO 4.0 Bridge Daemon (Port 18787) is active! (Phase: $($status.phase))" -ForegroundColor Green
} catch {
    Write-Warning "Waiting for Bridge Daemon on port 18787..."
}
