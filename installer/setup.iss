; Inno Setup 6.3+ script. Build with:  ISCC installer\setup.iss
; Needs administrator rights ONLY because it (a) copies files to Program Files and
; (b) registers a COM/TSF component in HKEY_CLASSES_ROOT (machine-wide). Nothing else.
[Setup]
AppName=Bangla Keyboard Switcher
AppVersion=1.0.0
AppPublisher=Independent project (not affiliated with Avro or Bijoy)
DefaultDirName={autopf}\BanglaKeyboardSwitcher
DefaultGroupName=Bangla Keyboard Switcher
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=BanglaKeyboardSwitcher-Setup
Compression=lzma2
SolidCompression=yes
UninstallDisplayName=Bangla Keyboard Switcher

[Files]
Source: "..\build\x64\Release\BanglaKeyboardTip.dll"; DestDir: "{app}"; DestName: "BanglaKeyboardTip64.dll"; Flags: ignoreversion restartreplace uninsrestartdelete
Source: "..\build\x86\Release\BanglaKeyboardTip.dll"; DestDir: "{app}"; DestName: "BanglaKeyboardTip32.dll"; Flags: ignoreversion restartreplace uninsrestartdelete
Source: "..\controller\publish\BanglaKeyboardSwitcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README_bn.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Bangla Keyboard Switcher"; Filename: "{app}\BanglaKeyboardSwitcher.exe"
Name: "{autodesktop}\Bangla Keyboard Switcher"; Filename: "{app}\BanglaKeyboardSwitcher.exe"

[Run]
; 64-bit and 32-bit apps each load their own DLL, so both are registered.
Filename: "{sys}\regsvr32.exe"; Parameters: "/s ""{app}\BanglaKeyboardTip64.dll"""; Flags: runhidden
Filename: "{syswow64}\regsvr32.exe"; Parameters: "/s ""{app}\BanglaKeyboardTip32.dll"""; Flags: runhidden
Filename: "{app}\BanglaKeyboardSwitcher.exe"; Description: "Open Bangla Keyboard Switcher"; Flags: postinstall nowait skipifsilent

[UninstallRun]
Filename: "{sys}\regsvr32.exe"; Parameters: "/u /s ""{app}\BanglaKeyboardTip64.dll"""; RunOnceId: "UnregTip64"; Flags: runhidden
Filename: "{syswow64}\regsvr32.exe"; Parameters: "/u /s ""{app}\BanglaKeyboardTip32.dll"""; RunOnceId: "UnregTip32"; Flags: runhidden
