Set WshShell = CreateObject("WScript.Shell")
userProfile = WshShell.ExpandEnvironmentStrings("%USERPROFILE%")
WshShell.Run """C:\Program Files\nodejs\node.exe"" """ & userProfile & "\.gemini\config\plugins\gemini-desktop\scripts\gemini_mcp_server.js"" --standalone", 0, False
