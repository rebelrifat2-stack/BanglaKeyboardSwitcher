#pragma once
#include "Globals.h"
#include "../core/engine.h"

class TextService : public ITfTextInputProcessorEx,
                    public ITfKeyEventSink,
                    public ITfCompositionSink,
                    public ITfThreadMgrEventSink {
public:
    TextService();
    ~TextService();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // ITfTextInputProcessor / Ex
    STDMETHODIMP Activate(ITfThreadMgr* ptim, TfClientId tid);
    STDMETHODIMP ActivateEx(ITfThreadMgr* ptim, TfClientId tid, DWORD flags);
    STDMETHODIMP Deactivate();

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr*) { return S_OK; }
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr*) { return S_OK; }
    STDMETHODIMP OnSetFocus(ITfDocumentMgr* pFocus, ITfDocumentMgr* pPrev);
    STDMETHODIMP OnPushContext(ITfContext*) { return S_OK; }
    STDMETHODIMP OnPopContext(ITfContext*) { return S_OK; }

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* pfEaten) { *pfEaten = FALSE; return S_OK; }
    STDMETHODIMP OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* pfEaten) { *pfEaten = FALSE; return S_OK; }
    STDMETHODIMP OnPreservedKey(ITfContext*, REFGUID, BOOL* pfEaten) { *pfEaten = FALSE; return S_OK; }

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ec, ITfComposition* pComposition);

private:
    enum Action { A_PASS, A_COMPOSE, A_STANDALONE, A_BACKSPACE, A_ESCAPE, A_COMMIT_PASS };
    Action Classify(UINT vk, char* outChar) const;
    HRESULT Run(ITfContext* ctx, DWORD flags, std::function<HRESULT(TfEditCookie)> fn);
    HRESULT UpdateComposition(TfEditCookie ec, ITfContext* ctx);
    HRESULT EndComposition(TfEditCookie ec, bool removeText);
    HRESULT InsertText(TfEditCookie ec, ITfContext* ctx, const std::wstring& text);
    void    ForceEnd();
    void    PickEngineFromActiveProfile();
    bool    Composing() const { return _comp != nullptr; }

    LONG                _ref;
    ITfThreadMgr*       _tm;
    TfClientId          _tid;
    DWORD               _tmCookie;
    const bks::Engine*  _engine;
    ITfComposition*     _comp;
    ITfContext*         _compCtx;
    std::string         _typed;      // keys typed in the current composition
};
