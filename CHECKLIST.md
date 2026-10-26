# Feature checklist (honest status)

Legend: ✅ implemented & tested here · 🟡 implemented, NOT yet compiled/run on Windows · ⬜ not implemented · ⛔ blocked

| # | Requirement | Status | Notes |
|---|---|---|---|
| 1 | Three-button UI, no text box, active-mode display, one-click switch | 🟡 | `controller/MainForm.cs` |
| 2 | System-wide typing via real TSF text service | 🟡 | `tsf/` — composition-based TIP; never compiled or run (no Windows/MSVC in my sandbox) |
| 3 | Avro-style phonetic engine | ✅ | `core/avro_engine.cpp`, 14 Avro tests pass on Linux |
| 3a | Official Avro code reuse | ⬜ | deliberately not reused (license not verified); independent engine instead |
| 3b | Dictionary / auto-correct / suggestions | ⬜ | differs from official Avro |
| 4 | Bijoy fixed layout → Unicode (prefix-kar reordering, ো ৌ, আ, hasanta-vowels) | ✅ engine / 🟡 in Windows | 30 Bijoy tests pass on Linux (46 total) |
| 4a | Bijoy keys vs official chart (3rd edition image from owner) | ✅ letters, kars, link layer, ৗ ৄ ঁ ৎ ঃ | digit-row link symbols (fractions etc.) and a few rare signs not implemented; `$`→৳ glyph hard to read |
| 4b | SutonnyMJ legacy ASCII output | ⬜ | not built: unverified tables risk silent corruption |
| 4c | Bundle SutonnyMJ font | ⬜ | proprietary; user must use their own licensed copy |
| 5 | Mode activation via TSF ActivateProfile; English = Windows US layout | 🟡 | `controller/Tsf.cs` |
| 5a | Persisted last-chosen mode | 🟡 | `%APPDATA%\BanglaKeyboardSwitcher\mode.txt` |
| 5b | Clear error if component missing | 🟡 | |
| 6 | Modular architecture | ✅ | core / tsf / controller / installer |
| 7 | Installer (admin only for install), uninstaller removing only own entries | 🟡 | `installer/setup.iss`; not built |
| 7b | Cloud build (GitHub Actions) so no local tools are needed | 🟡 | `.github/workflows/build.yml`, never run |
| 7a | Code signing | ⬜ | needs paid certificate; SmartScreen warning expected |
| 8 | Engine unit tests | ✅ | 46/46 run on Linux g++ |
| 8a | Tests in Notepad/Word/Excel/Chrome, restart, uninstall | ⬜ | **not run** — see docs/TESTING.md |
| 9 | Bangla README | ✅ | `README_bn.md` |
| 10 | Start with Windows | ⬜ | optional, deferred |
| 11 | ARM64 Windows | ⛔ | x64/x86 only |
