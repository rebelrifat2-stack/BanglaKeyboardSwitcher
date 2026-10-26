# Testing — what was run and what was NOT

## Actually run (Linux, g++ 13, `tests/run_tests.sh`) — 46/46 PASS
Avro: `ami banglay likhi`→আমি বাংলায় লিখি, kolom, bondhu, shikkha, bangladesh, prem, bakyo, bijNGan, ghor, digits+।, চাঁদ, ঐ, স্বপ্ন, backspace re-conversion.
Bijoy: `Ffdm`→আমি, বাংলা, লিখি (prefix i-kar reorder), ক্ষ, ইস্কুল, পৌষ, পো, র্ম, ব্র/ব্য, এ, digits, য়, backspace re-conversion.

## NOT run (needs a Windows 10/11 machine) — please run and report
Tick each box and send me failures. None of these has been executed by me.
- [ ] `build.bat` completes (x64 + Win32 DLLs, controller, installer)
- [ ] Install; confirm UAC prompt and no other prompts
- [ ] Settings → Language shows Bengali (Bangladesh) with two "Bangla Keyboard Switcher" keyboards
- [ ] Launch switcher → click অভ্র; Notepad: `ami banglay likhi ` → আমি বাংলায় লিখি
- [ ] Backspace mid-word; Esc; punctuation; digits; `kkh`, `jNG`, `ng`
- [ ] Click বিজয়; Notepad: `Ffdm`, `hfQVf`, `dVdJ`, `jgN`, `gdngjsV`
- [ ] Click Regular English; type normally; Ctrl+C/V still work
- [ ] Repeat in Word, Excel (cell + formula bar), Chrome (address bar, Google Docs, a web textarea), WhatsApp/Telegram desktop
- [ ] Switching modes mid-document does not alter existing text; mode change with an open composition commits it
- [ ] Close the switcher → typing still works
- [ ] 32-bit app test (e.g. old 32-bit Notepad++) — verifies the 32-bit DLL registration
- [ ] Reboot → previously chosen profile still active; switcher shows last choice
- [ ] Uninstall → keyboards disappear; `reg query HKCR\CLSID\{B7A3C5D1-5E2F-4A9B-9C31-7D4E8F20A6B1}` finds nothing; other keyboards intact
- [ ] Not tested at all: SutonnyMJ legacy output (not implemented)
