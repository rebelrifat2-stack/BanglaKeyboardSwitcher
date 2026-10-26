@echo off
REM DEVELOPER USE: register freshly built DLLs without the installer. Run as Administrator.
%SystemRoot%\System32\regsvr32.exe "%~dp0build\x64\Release\BanglaKeyboardTip.dll"
%SystemRoot%\SysWOW64\regsvr32.exe "%~dp0build\x86\Release\BanglaKeyboardTip.dll"
