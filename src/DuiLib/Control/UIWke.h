#ifndef __UIWKE_H__
#define __UIWKE_H__
#pragma once

#include <vector>

namespace wke {
    class CWebView;
}
typedef wke::CWebView* wkeWebView;
typedef void(*PFN_OnDocumentReadyNotify)(DWORD context);
namespace DuiLib {
    class CWkeUI;
    class RenderGDI;

    class CWkeWnd : public CWindowWnd {
        friend class CWkeUI;
    public:
        CWkeWnd(CPaintManagerUI* pParentPM);
        ~CWkeWnd();

        void RegisterDocumentReadyCallback(PFN_OnDocumentReadyNotify cb, DWORD context);
        void SetUrl(const base::String& strUrl);
        void SetBkImage(const base::String& strBkImage);
        void ShowDevTools();
        void GoForward();
        void GoBack();
        void Reload();
        VARIANT CallJavascriptFunction(const std::string &strFuncName, const std::vector<VARIANT> &vars);

        void Init(CWkeUI* pOwner);
        LPCTSTR GetWindowClassName() const;
        void OnFinalMessage(HWND hWnd);

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

        static void OnDocumentReadyCallback(wkeWebView webView, void* param);
        static void OnPaintUpdatedCallback(wkeWebView webView, void* param, const HDC hdc, int x, int y, int cx, int cy);
    protected:
        bool SetCursorInfoTypeByCache(HWND hWnd);
        CWkeUI* m_pOwner;
        CPaintManagerUI* m_pParentPM;
        wkeWebView	 m_pWebView;
        base::String    m_strUrl;
        base::String    m_strBkImage;
        TDrawInfo     m_diBkImage;
        bool		  m_bInit;
        int           m_iCursorInfoType;
        UINT_PTR      m_pTimer;
        RenderGDI*    m_pRender;

        PFN_OnDocumentReadyNotify m_pReadyCB;
        DWORD         m_dwContext;
    };

    class CWkeUI :public CControlUI {
        typedef std::string(*PFN_OnJSNotify)(const std::vector<VARIANT> &vars, DWORD context);
        friend class CWkeWnd;
        DECLARE_DUICONTROL(CWkeUI)
    public:
        CWkeUI(void);
        ~CWkeUI(void);

        static void WkeInit();
        static void WkeShutdown();
        static void RegisterJSNotifyCallback(PFN_OnJSNotify cb, DWORD context);

        void RegisterDocumentReadyCallback(PFN_OnDocumentReadyNotify cb, DWORD context);

        LPCTSTR	GetClass() const override;
        LPVOID	GetInterface(LPCTSTR pstrName) override;

        void SetVisible(bool bVisible) override;
        void DoInit() override;
        void DoEvent(TEventUI& event) override;
        void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;
        void SetPos(RECT rc, bool bNeedInvalidate = true) override;
        void SetURL(const base::String& strValue);
        void SetBkImage(const base::String& strValue);
        VARIANT CallJavascriptFunction(const std::string &strFuncName, const std::vector<VARIANT> &vars);

        void GoBack();
        void GoForward();
        void Reload();
        void ShowDevTools();
    public:
        static PFN_OnJSNotify m_pfnOnJSNotify;
        static DWORD m_dwContext;
    protected:
        CWkeWnd *m_pWindow;
        base::String  m_strUrl;
        base::String  m_strBkImage;
    };
}

#endif // !__UIWKE_H__