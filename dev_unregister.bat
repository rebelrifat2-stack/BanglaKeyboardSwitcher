@echo off
REM DEVELOPER USE: remove the registrations again. Run as Administrator.
%SystemRoot%\System32\regsvr32.exe /u "%~dp0build\x64\Release\BanglaKeyboardTip.dll"
%SystemRoot%\SysWOW64\regsvr32.exe /u "%~dp0build\x86\Release\BanglaKeyboardTip.dll"
