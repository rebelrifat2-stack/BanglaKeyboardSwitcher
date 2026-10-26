@echo off
REM Builds everything: engine tests, 64-bit + 32-bit TSF DLLs, controller, installer.
REM Run from "x64 Native Tools Command Prompt for VS 2022" (or any shell where cl.exe works).
setlocal
cmake -S . -B build\x64 -A x64            || goto :err
cmake --build build\x64 --config Release  || goto :err
cmake -S . -B build\x86 -A Win32          || goto :err
cmake --build build\x86 --config Release  || goto :err
echo === Engine unit tests ===
build\x64\Release\engine_tests.exe        || goto :err
dotnet publish controller\BanglaSwitcher.csproj -c Release -o controller\publish || goto :err
if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" (
  "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" installer\setup.iss || goto :err
) else (
  echo Inno Setup not found - installer NOT built. Install it: winget install JRSoftware.InnoSetup
)
echo BUILD OK. Installer: dist\BanglaKeyboardSwitcher-Setup.exe
exit /b 0
:err
echo BUILD FAILED
exit /b 1
