#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <msctf.h>
#include <string>
#include <functional>

extern HINSTANCE g_hInst;
extern LONG      g_dllRef;

// ---- Our identifiers (fixed; the C# controller uses the same values) ----
extern const GUID CLSID_BanglaSwitcherTip;   // {B7A3C5D1-5E2F-4A9B-9C31-7D4E8F20A6B1}
extern const GUID GUID_ProfileAvro;          // {2F6D1B84-93C7-4E55-A1D2-0C8B7E3F4A19}
extern const GUID GUID_ProfileBijoy;         // {8E41C7A2-6B35-4D08-B9F3-1A5D2C7E9B60}

static const LANGID BKS_LANGID = 0x0845;     // bn-BD (Bengali, Bangladesh)

// TSF category GUIDs, copied here so we do not depend on which lib defines them.
static const GUID BKS_TFCAT_TIP_KEYBOARD =
    {0x34745c63,0xb2f0,0x4784,{0x8b,0x67,0x5e,0x12,0xc8,0x70,0x1a,0x31}};
static const GUID BKS_TFCAT_TIPCAP_IMMERSIVESUPPORT =
    {0x13a016df,0x560b,0x46cd,{0x94,0x7a,0x4c,0x3a,0xf1,0xe0,0xe3,0x5d}};
static const GUID BKS_TFCAT_TIPCAP_SYSTRAYSUPPORT =
    {0x25504fb4,0x7bab,0x4bc1,{0x9c,0x69,0xcf,0x81,0x89,0x0f,0x0e,0xf5}};

HRESULT RegisterTip();
HRESULT UnregisterTip();
