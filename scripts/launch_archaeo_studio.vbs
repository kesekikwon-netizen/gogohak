Option Explicit
Dim fso, shell, scriptDir, pluginDir, nodeCmd

Set fso = CreateObject("Scripting.FileSystemObject")
Set shell = CreateObject("WScript.Shell")

scriptDir = fso.GetParentFolderName(WScript.ScriptFullName)
pluginDir = fso.GetParentFolderName(scriptDir)

' Launch ONLY the single ARCHAEO STUDIO Standalone Window & Bridge Server silently (no extra empty WordCraft window)
nodeCmd = "node """ & pluginDir & "\scripts\gemini_mcp_server.js"" --standalone"
shell.Run nodeCmd, 0, False
