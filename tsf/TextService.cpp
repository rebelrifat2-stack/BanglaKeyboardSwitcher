#include "TextService.h"
#include <new>

// ---------------------------------------------------------------- edit session
namespace {
class EditSession : public ITfEditSession {
public:
    explicit EditSession(std::function<HRESULT(TfEditCookie)> fn) : _ref(1), _fn(std::move(fn)) {}
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfEditSession)) {
            *ppv = static_cast<ITfEditSession*>(this); AddRef(); return S_OK;
        }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_ref); }
    STDMETHODIMP_(ULONG) Release() { LONG r = InterlockedDecrement(&_ref); if (!r) delete this; return r; }
    STDMETHODIMP DoEditSession(TfEditCookie ec) { return _fn(ec); }
private:
    LONG _ref;
    std::function<HRESULT(TfEditCookie)> _fn;
};

// Physical-key -> the ASCII a US-QWERTY keyboard would give (Shift only).
char VkToAscii(UINT vk, bool shift) {
    if (vk >= 'A' && vk <= 'Z') return shift ? (char)vk : (char)(vk + 32);
    if (vk >= '0' && vk <= '9') { static const char sh[] = ")!@#$%^&*("; return shift ? sh[vk - '0'] : (char)vk; }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) return (char)('0' + (vk - VK_NUMPAD0));
    switch (vk) {
    case VK_OEM_1:      return shift ? ':' : ';';
    case VK_OEM_PLUS:   return shift ? '+' : '=';
    case VK_OEM_COMMA:  return shift ? '<' : ',';
    case VK_OEM_MINUS:  return shift ? '_' : '-';
    case VK_OEM_PERIOD: return shift ? '>' : '.';
    case VK_OEM_2:      return shift ? '?' : '/';
    case VK_OEM_3:      return shift ? '~' : '`';
    case VK_OEM_4:      return shift ? '{' : '[';
    case VK_OEM_5:      return shift ? '|' : '\\';
    case VK_OEM_6:      return shift ? '}' : ']';
    case VK_OEM_7:      return shift ? '"' : '\'';
    }
    return 0;
}

bool IsModifierVk(UINT vk) {
    switch (vk) {
    case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL:
    case VK_MENU: case VK_LMENU: case VK_RMENU: case VK_CAPITAL: case VK_NUMLOCK: case VK_SCROLL:
    case VK_LWIN: case VK_RWIN: return true;
    }
    return false;
}
bool Down(int vk) { return (GetKeyState(vk) & 0x8000) != 0; }
} // namespace

// ---------------------------------------------------------------- lifetime
TextService::TextService()
    : _ref(1), _tm(nullptr), _tid(TF_CLIENTID_NULL), _tmCookie(TF_INVALID_COOKIE),
      _engine(&bks::AvroEngine()), _comp(nullptr), _compCtx(nullptr) {
    InterlockedIncrement(&g_dllRef);
}
TextService::~TextService() { InterlockedDecrement(&g_dllRef); }

STDMETHODIMP TextService::QueryInterface(REFIID riid, void** ppv) {
    if (!ppv) return E_INVALIDARG;
    *ppv = nullptr;
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor) || IsEqualIID(riid, IID_ITfTextInputProcessorEx))
        *ppv = static_cast<ITfTextInputProcessorEx*>(this);
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))        *ppv = static_cast<ITfKeyEventSink*>(this);
    else if (IsEqualIID(riid, IID_ITfCompositionSink))     *ppv = static_cast<ITfCompositionSink*>(this);
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))  *ppv = static_cast<ITfThreadMgrEventSink*>(this);
    if (!*ppv) return E_NOINTERFACE;
    AddRef(); return S_OK;
}
STDMETHODIMP_(ULONG) TextService::AddRef() { return InterlockedIncrement(&_ref); }
STDMETHODIMP_(ULONG) TextService::Release() { LONG r = InterlockedDecrement(&_ref); if (!r) delete this; return r; }

// ---------------------------------------------------------------- activation
void TextService::PickEngineFromActiveProfile() {
    _engine = &bks::AvroEngine();                       // default
    ITfInputProcessorProfileMgr* pm = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_ITfInputProcessorProfileMgr, (void**)&pm)) && pm) {
        TF_INPUTPROCESSORPROFILE prof = {};
        if (SUCCEEDED(pm->GetActiveProfile(BKS_TFCAT_TIP_KEYBOARD, &prof)) &&
            IsEqualGUID(prof.clsid, CLSID_BanglaSwitcherTip) &&
            IsEqualGUID(prof.guidProfile, GUID_ProfileBijoy)) {
            _engine = &bks::BijoyEngine();
        }
        pm->Release();
    }
}

STDMETHODIMP TextService::Activate(ITfThreadMgr* ptim, TfClientId tid) { return ActivateEx(ptim, tid, 0); }

STDMETHODIMP TextService::ActivateEx(ITfThreadMgr* ptim, TfClientId tid, DWORD) {
    _tm = ptim; _tm->AddRef(); _tid = tid;
    PickEngineFromActiveProfile();

    ITfKeystrokeMgr* km = nullptr;
    if (SUCCEEDED(_tm->QueryInterface(IID_ITfKeystrokeMgr, (void**)&km)) && km) {
        HRESULT hr = km->AdviseKeyEventSink(_tid, static_cast<ITfKeyEventSink*>(this), TRUE);
        km->Release();
        if (FAILED(hr)) { Deactivate(); return hr; }
    }
    ITfSource* src = nullptr;
    if (SUCCEEDED(_tm->QueryInterface(IID_ITfSource, (void**)&src)) && src) {
        src->AdviseSink(IID_ITfThreadMgrEventSink, static_cast<ITfThreadMgrEventSink*>(this), &_tmCookie);
        src->Release();
    }
    return S_OK;
}

STDMETHODIMP TextService::Deactivate() {
    ForceEnd(); PickEngineFromActiveProfile();
    if (_tm) {
        ITfKeystrokeMgr* km = nullptr;
        if (SUCCEEDED(_tm->QueryInterface(IID_ITfKeystrokeMgr, (void**)&km)) && km) { km->UnadviseKeyEventSink(_tid); km->Release(); }
        if (_tmCookie != TF_INVALID_COOKIE) {
            ITfSource* src = nullptr;
            if (SUCCEEDED(_tm->QueryInterface(IID_ITfSource, (void**)&src)) && src) { src->UnadviseSink(_tmCookie); src->Release(); }
            _tmCookie = TF_INVALID_COOKIE;
        }
        _tm->Release(); _tm = nullptr;
    }
    if (_comp) { _comp->Release(); _comp = nullptr; }
    if (_compCtx) { _compCtx->Release(); _compCtx = nullptr; }
    _typed.clear(); _tid = TF_CLIENTID_NULL;
    return S_OK;
}

STDMETHODIMP TextService::OnSetFocus(ITfDocumentMgr*, ITfDocumentMgr*) { ForceEnd(); PickEngineFromActiveProfile(); return S_OK; }
STDMETHODIMP TextService::OnSetFocus(BOOL) { return S_OK; }

STDMETHODIMP TextService::OnCompositionTerminated(TfEditCookie, ITfComposition* pComp) {
    if (pComp == _comp) {
        _comp->Release(); _comp = nullptr;
        if (_compCtx) { _compCtx->Release(); _compCtx = nullptr; }
        _typed.clear();
    }
    return S_OK;
}

// ---------------------------------------------------------------- edit helpers
HRESULT TextService::Run(ITfContext* ctx, DWORD flags, std::function<HRESULT(TfEditCookie)> fn) {
    EditSession* es = new (std::nothrow) EditSession(std::move(fn));
    if (!es) return E_OUTOFMEMORY;
    HRESULT hrSession = E_FAIL;
    HRESULT hr = ctx->RequestEditSession(_tid, es, flags, &hrSession);
    es->Release();
    return SUCCEEDED(hr) ? hrSession : hr;
}

static void CaretToEnd(TfEditCookie ec, ITfContext* ctx, ITfRange* range) {
    ITfRange* end = nullptr;
    if (SUCCEEDED(range->Clone(&end)) && end) {
        end->Collapse(ec, TF_ANCHOR_END);
        TF_SELECTION sel = {};
        sel.range = end; sel.style.ase = TF_AE_NONE; sel.style.fInterimChar = FALSE;
        ctx->SetSelection(ec, 1, &sel);
        end->Release();
    }
}

HRESULT TextService::UpdateComposition(TfEditCookie ec, ITfContext* ctx) {
    const std::wstring text = _engine->Convert(_typed);
    ITfRange* range = nullptr;

    if (!_comp) {
        ITfInsertAtSelection* ias = nullptr;
        HRESULT hr = ctx->QueryInterface(IID_ITfInsertAtSelection, (void**)&ias);
        if (FAILED(hr)) return hr;
        hr = ias->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, nullptr, 0, &range);
        ias->Release();
        if (FAILED(hr) || !range) return FAILED(hr) ? hr : E_FAIL;

        ITfContextComposition* cc = nullptr;
        hr = ctx->QueryInterface(IID_ITfContextComposition, (void**)&cc);
        if (SUCCEEDED(hr)) {
            hr = cc->StartComposition(ec, range, static_cast<ITfCompositionSink*>(this), &_comp);
            cc->Release();
        }
        range->Release(); range = nullptr;
        if (FAILED(hr) || !_comp) { _comp = nullptr; return FAILED(hr) ? hr : E_FAIL; }
        _compCtx = ctx; _compCtx->AddRef();
    }

    HRESULT hr = _comp->GetRange(&range);
    if (FAILED(hr) || !range) return FAILED(hr) ? hr : E_FAIL;
    hr = range->SetText(ec, 0, text.c_str(), (LONG)text.size());
    if (SUCCEEDED(hr)) CaretToEnd(ec, ctx, range);
    range->Release();
    return hr;
}

HRESULT TextService::EndComposition(TfEditCookie ec, bool removeText) {
    if (!_comp) return S_OK;
    if (removeText) {
        ITfRange* range = nullptr;
        if (SUCCEEDED(_comp->GetRange(&range)) && range) { range->SetText(ec, 0, L"", 0); range->Release(); }
    }
    HRESULT hr = _comp->EndComposition(ec);
    _comp->Release(); _comp = nullptr;
    if (_compCtx) { _compCtx->Release(); _compCtx = nullptr; }
    _typed.clear();
    return hr;
}

HRESULT TextService::InsertText(TfEditCookie ec, ITfContext* ctx, const std::wstring& text) {
    ITfInsertAtSelection* ias = nullptr;
    HRESULT hr = ctx->QueryInterface(IID_ITfInsertAtSelection, (void**)&ias);
    if (FAILED(hr)) return hr;
    ITfRange* range = nullptr;
    hr = ias->InsertTextAtSelection(ec, 0, text.c_str(), (LONG)text.size(), &range);
    ias->Release();
    if (SUCCEEDED(hr) && range) { CaretToEnd(ec, ctx, range); range->Release(); }
    return hr;
}

void TextService::ForceEnd() {
    if (!_comp || !_compCtx) { _typed.clear(); return; }
    ITfContext* ctx = _compCtx; ctx->AddRef();
    AddRef();
    HRESULT hr = Run(ctx, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, [this](TfEditCookie ec) {
        HRESULT r = EndComposition(ec, false);
        Release();
        return r;
    });
    if (FAILED(hr)) {                       // edit session never ran: drop our state safely
        Release();
        if (_comp) { _comp->Release(); _comp = nullptr; }
        if (_compCtx) { _compCtx->Release(); _compCtx = nullptr; }
        _typed.clear();
    }
    ctx->Release();
}

// ---------------------------------------------------------------- key handling
TextService::Action TextService::Classify(UINT vk, char* outChar) const {
    *outChar = 0;
    const bool composing = Composing();
    const bool ctrlOrAlt = Down(VK_CONTROL) || Down(VK_MENU);   // includes AltGr
    if (ctrlOrAlt) return composing ? A_COMMIT_PASS : A_PASS;
    if (vk == VK_BACK)   return composing ? A_BACKSPACE : A_PASS;
    if (vk == VK_ESCAPE) return composing ? A_ESCAPE : A_PASS;

    const char c = VkToAscii(vk, Down(VK_SHIFT));
    if (!c) return composing ? A_COMMIT_PASS : A_PASS;           // space, enter, tab, arrows ...
    *outChar = c;
    if (_engine->IsComposable(c)) return A_COMPOSE;
    if (!_engine->Standalone(c).empty()) return A_STANDALONE;
    return composing ? A_COMMIT_PASS : A_PASS;
}

STDMETHODIMP TextService::OnTestKeyDown(ITfContext*, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    *pfEaten = FALSE;
    if (!_engine || IsModifierVk((UINT)wParam)) return S_OK;
    char c; Action a = Classify((UINT)wParam, &c);
    // COMMIT_PASS must reach OnKeyDown so the composition can be committed first.
    *pfEaten = (a != A_PASS);
    return S_OK;
}

STDMETHODIMP TextService::OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM, BOOL* pfEaten) {
    *pfEaten = FALSE;
    if (!_engine || IsModifierVk((UINT)wParam)) return S_OK;

    if (_compCtx && _compCtx != pic) ForceEnd(); PickEngineFromActiveProfile();               // focus moved to another context

    char c; Action a = Classify((UINT)wParam, &c);
    switch (a) {
    case A_PASS:
        return S_OK;
    case A_COMMIT_PASS:
        Run(pic, TF_ES_SYNC | TF_ES_READWRITE, [this](TfEditCookie ec) { return EndComposition(ec, false); });
        return S_OK;                                           // key still goes to the application
    case A_ESCAPE:
        Run(pic, TF_ES_SYNC | TF_ES_READWRITE, [this](TfEditCookie ec) { return EndComposition(ec, true); });
        *pfEaten = TRUE; return S_OK;
    case A_BACKSPACE:
        if (!_typed.empty()) _typed.pop_back();
        if (_typed.empty())
            Run(pic, TF_ES_SYNC | TF_ES_READWRITE, [this](TfEditCookie ec) { return EndComposition(ec, true); });
        else
            Run(pic, TF_ES_SYNC | TF_ES_READWRITE, [this, pic](TfEditCookie ec) { return UpdateComposition(ec, pic); });
        *pfEaten = TRUE; return S_OK;
    case A_COMPOSE: {
        _typed.push_back(c);
        HRESULT hr = Run(pic, TF_ES_SYNC | TF_ES_READWRITE, [this, pic](TfEditCookie ec) { return UpdateComposition(ec, pic); });
        if (FAILED(hr)) { _typed.clear(); return S_OK; }       // let the app have the key rather than lose it
        *pfEaten = TRUE; return S_OK;
    }
    case A_STANDALONE: {
        const std::wstring s = _engine->Standalone(c);
        Run(pic, TF_ES_SYNC | TF_ES_READWRITE, [this, pic, s](TfEditCookie ec) {
            EndComposition(ec, false);
            return InsertText(ec, pic, s);
        });
        *pfEaten = TRUE; return S_OK;
    }
    }
    return S_OK;
}
