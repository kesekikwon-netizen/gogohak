Option Explicit
Dim fso, WshShell, scriptDir, serverScript, userProfile, nodeExe
Set fso = CreateObject("Scripting.FileSystemObject")
Set WshShell = CreateObject("WScript.Shell")
scriptDir = fso.GetParentFolderName(WScript.ScriptFullName)
serverScript = scriptDir & "\gemini_mcp_server.js"
If Not fso.FileExists(serverScript) Then
  userProfile = WshShell.ExpandEnvironmentStrings("%USERPROFILE%")
  serverScript = userProfile & "\.gemini\config\plugins\gemini-desktop\scripts\gemini_mcp_server.js"
End If
nodeExe = "node"
If fso.FileExists("C:\Program Files\nodejs\node.exe") Then
  nodeExe = """C:\Program Files\nodejs\node.exe"""
End If
WshShell.Run nodeExe & " """ & serverScript & """ --standalone", 0, False
