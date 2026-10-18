Option Explicit
Dim fso, shell, scriptDir, pluginDir, nodeCmd, wordcraftExe

Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")

scriptDir = fso.GetParentFolderName(WScript.ScriptFullName)
pluginDir = fso.GetParentFolderName(scriptDir)

' 1. Start the ARCHAEO STUDIO Standalone App & Bridge Server silently (no black console window)
nodeCmd = "node """ & pluginDir & "\scripts\gemini_mcp_server.js"" --standalone"
shell.Run nodeCmd, 0, False

' 2. Also launch WordCraft (with the docked ARCHAEO STUDIO left panel) if desired
wordcraftExe = pluginDir & "\bin\wordcraft\wordcraft-0.7.0-windows-x64-portable\wordcraft.exe"
If fso.FileExists(wordcraftExe) Then
    shell.Run """" & wordcraftExe & """", 1, False
End If
