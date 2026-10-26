# Architecture

```
 ┌────────────────────────┐   ITfInputProcessorProfileMgr::ActivateProfile
 │ BanglaKeyboardSwitcher │ ───────────────────────────────────────────────┐
 │ (C# WinForms, 3 btns)  │                                                 ▼
 └────────────────────────┘                          Windows Text Services Framework
                                                                   │ loads, per app
                                              ┌────────────────────▼─────────────────┐
                                              │ BanglaKeyboardTip{64,32}.dll (C++)    │
                                              │  TextService : ITfTextInputProcessorEx│
                                              │   key sink → composition (pre-edit)   │
                                              │   core/avro_engine + core/bijoy_engine│
                                              └───────────────────────────────────────┘
```

## Why TSF
A text service is the supported way to inject text into arbitrary Windows apps, with composition, undo
and correct behaviour in Word/Chrome/etc. Keyboard hooks + SendInput, or a plain window, are not equivalent.

## Key decisions
* **Two profiles, one CLSID** (`bn-BD`): Avro and Bijoy. English = Windows' own English (US) layout.
* **Physical-key model**: the TIP converts VK + Shift to the US-QWERTY ASCII char and feeds that to the engine, so results do not depend on the currently selected layout (Bijoy assumes QWERTY positions).
* **Composition**: letters accumulate in a pre-edit buffer; the whole buffer is re-converted on each key (pure function), so Backspace can never corrupt neighbouring text. Space/Enter/Tab/arrows/Ctrl-combos commit and then pass through. Esc cancels.
* **Per-user list**: the controller calls `InstallLayoutOrTip("0845:{CLSID}{PROFILE}")` (no admin) so the profiles appear for the user.
* **Admin only at install**: HKCR\CLSID + Program Files. Unregister removes only our CLSID key and profiles.
* Static CRT (`/MT`): the DLL loads into other processes and must not need the VC++ redistributable.

## Known differences / limits
* Avro engine: independent implementation. No dictionary, no auto-correct, no suggestion window, `ee`/`oo` handling and `w`/`y` rules are from memory of Avro conventions and may differ in edge cases.
* Bijoy: Unicode output only; independent vowels typed as hasanta + sign key (the chart's "link" layers; only অ and ও have their own keys); layout checked against the official chart image supplied by the project owner (digit-row link symbols omitted).
* No display-attribute provider, so some apps show the composition without an underline.
* Elevated (run-as-administrator) apps and some secure desktops do not accept normal-user TSF switching.
* Licensing: Bijoy's layout ownership is disputed publicly (Ekushey's notes vs. its adoption as the national layout by the Bangladesh Computer Council). This project reimplements a keyboard layout from public knowledge, ships no Bijoy/SutonnyMJ binaries or fonts, and claims no affiliation.
