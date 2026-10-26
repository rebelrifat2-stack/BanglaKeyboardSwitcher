#include "Globals.h"
#include "TextService.h"
#include <new>

HINSTANCE g_hInst = nullptr;
LONG      g_dllRef = 0;

const GUID CLSID_BanglaSwitcherTip =
    {0xb7a3c5d1,0x5e2f,0x4a9b,{0x9c,0x31,0x7d,0x4e,0x8f,0x20,0xa6,0xb1}};
const GUID GUID_ProfileAvro =
    {0x2f6d1b84,0x93c7,0x4e55,{0xa1,0xd2,0x0c,0x8b,0x7e,0x3f,0x4a,0x19}};
const GUID GUID_ProfileBijoy =
    {0x8e41c7a2,0x6b35,0x4d08,{0xb9,0xf3,0x1a,0x5d,0x2c,0x7e,0x9b,0x60}};

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) { g_hInst = h; DisableThreadLibraryCalls(h); }
    return TRUE;
}

// ------------------------------------------------------------ class factory
class Factory : public IClassFactory {
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory)) { *ppv = this; AddRef(); return S_OK; }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return 2; }       // static object
    STDMETHODIMP_(ULONG) Release() { return 1; }
    STDMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** ppv) {
        if (outer) return CLASS_E_NOAGGREGATION;
        TextService* ts = new (std::nothrow) TextService();
        if (!ts) return E_OUTOFMEMORY;
        HRESULT hr = ts->QueryInterface(riid, ppv);
        ts->Release();
        return hr;
    }
    STDMETHODIMP LockServer(BOOL lock) {
        if (lock) InterlockedIncrement(&g_dllRef); else InterlockedDecrement(&g_dllRef);
        return S_OK;
    }
};
static Factory g_factory;

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv) {
    if (IsEqualCLSID(rclsid, CLSID_BanglaSwitcherTip)) return g_factory.QueryInterface(riid, ppv);
    *ppv = nullptr; return CLASS_E_CLASSNOTAVAILABLE;
}
STDAPI DllCanUnloadNow() { return g_dllRef == 0 ? S_OK : S_FALSE; }

// ------------------------------------------------------------ registration
static std::wstring GuidStr(const GUID& g) {
    wchar_t b[64]; StringFromGUID2(g, b, 64); return b;
}

static HRESULT WriteString(HKEY root, const std::wstring& path, const wchar_t* name, const std::wstring& val) {
    HKEY k; LONG r = RegCreateKeyExW(root, path.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &k, nullptr);
    if (r != ERROR_SUCCESS) return HRESULT_FROM_WIN32(r);
    r = RegSetValueExW(k, name, 0, REG_SZ, (const BYTE*)val.c_str(), (DWORD)((val.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(k);
    return HRESULT_FROM_WIN32(r);
}

HRESULT RegisterTip() {
    wchar_t dll[MAX_PATH];
    if (!GetModuleFileNameW(g_hInst, dll, MAX_PATH)) return HRESULT_FROM_WIN32(GetLastError());
    const std::wstring clsid = GuidStr(CLSID_BanglaSwitcherTip);

    // 1) COM registration (HKCR\CLSID\{...}) - needs administrator rights.
    HRESULT hr = WriteString(HKEY_CLASSES_ROOT, L"CLSID\\" + clsid, nullptr, L"Bangla Keyboard Switcher");
    if (FAILED(hr)) return hr;
    hr = WriteString(HKEY_CLASSES_ROOT, L"CLSID\\" + clsid + L"\\InprocServer32", nullptr, dll);
    if (FAILED(hr)) return hr;
    hr = WriteString(HKEY_CLASSES_ROOT, L"CLSID\\" + clsid + L"\\InprocServer32", L"ThreadingModel", L"Apartment");
    if (FAILED(hr)) return hr;

    // 2) TSF profiles
    ITfInputProcessorProfileMgr* pm = nullptr;
    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfileMgr, (void**)&pm);
    if (FAILED(hr)) return hr;
    const wchar_t* dAvro  = L"Bangla Keyboard Switcher - Avro Phonetic";
    const wchar_t* dBijoy = L"Bangla Keyboard Switcher - Bijoy";
    hr = pm->RegisterProfile(CLSID_BanglaSwitcherTip, BKS_LANGID, GUID_ProfileAvro,
                             dAvro, (ULONG)wcslen(dAvro), nullptr, 0, 0, nullptr, 0, TRUE, 0);
    if (SUCCEEDED(hr))
        hr = pm->RegisterProfile(CLSID_BanglaSwitcherTip, BKS_LANGID, GUID_ProfileBijoy,
                                 dBijoy, (ULONG)wcslen(dBijoy), nullptr, 0, 0, nullptr, 0, TRUE, 0);
    pm->Release();
    if (FAILED(hr)) return hr;

    // 3) Categories: keyboard TIP, usable in Store/UWP apps, tray-icon support
    ITfCategoryMgr* cm = nullptr;
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&cm);
    if (FAILED(hr)) return hr;
    const GUID* cats[] = { &BKS_TFCAT_TIP_KEYBOARD, &BKS_TFCAT_TIPCAP_IMMERSIVESUPPORT, &BKS_TFCAT_TIPCAP_SYSTRAYSUPPORT };
    for (const GUID* c : cats) {
        hr = cm->RegisterCategory(CLSID_BanglaSwitcherTip, *c, CLSID_BanglaSwitcherTip);
        if (FAILED(hr)) break;
    }
    cm->Release();
    return hr;
}

HRESULT UnregisterTip() {
    ITfCategoryMgr* cm = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&cm)) && cm) {
        const GUID* cats[] = { &BKS_TFCAT_TIP_KEYBOARD, &BKS_TFCAT_TIPCAP_IMMERSIVESUPPORT, &BKS_TFCAT_TIPCAP_SYSTRAYSUPPORT };
        for (const GUID* c : cats) cm->UnregisterCategory(CLSID_BanglaSwitcherTip, *c, CLSID_BanglaSwitcherTip);
        cm->Release();
    }
    ITfInputProcessorProfileMgr* pm = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_ITfInputProcessorProfileMgr, (void**)&pm)) && pm) {
        pm->UnregisterProfile(CLSID_BanglaSwitcherTip, BKS_LANGID, GUID_ProfileAvro, 0);
        pm->UnregisterProfile(CLSID_BanglaSwitcherTip, BKS_LANGID, GUID_ProfileBijoy, 0);
        pm->Release();
    }
    // Only our own CLSID key is removed - nothing else.
    RegDeleteTreeW(HKEY_CLASSES_ROOT, (L"CLSID\\" + GuidStr(CLSID_BanglaSwitcherTip)).c_str());
    return S_OK;
}

STDAPI DllRegisterServer()   { return RegisterTip(); }
STDAPI DllUnregisterServer() { return UnregisterTip(); }
