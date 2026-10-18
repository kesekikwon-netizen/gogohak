Set WshShell = CreateObject("WScript.Shell")
userProfile = WshShell.ExpandEnvironmentStrings("%USERPROFILE%")
WshShell.Run """C:\Program Files\nodejs\node.exe"" """ & userProfile & "\.gemini\config\plugins\gemini-desktop\scripts\gemini_mcp_server.js"" --off", 0, True
