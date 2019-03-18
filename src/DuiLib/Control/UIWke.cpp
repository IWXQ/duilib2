#include "Stdafx.h"
#include <io.h>
#include "../../3rd/wke/wke.h"
#include "Utils/Utils.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include <GdiPlus.h>

namespace DuiLib {
#define WUM_REFRESH_TIMER (1)


    class RenderGDI {
      public:
        RenderGDI()
            : m_hView(NULL)
            , m_hDC(NULL)
            , m_hBitmap(NULL)
            , m_pixels(NULL)
            , m_width(0)
            , m_height(0)
            , m_bInit(false) {
        }

        ~RenderGDI() {
            if (m_hDC)
                DeleteDC(m_hDC);

            if (m_hBitmap)
                DeleteObject(m_hBitmap);
        }

        virtual bool init(HWND hView) {
            m_hView = hView;
            m_hDC = CreateCompatibleDC(0);

            RECT rect;
            GetClientRect(hView, &rect);
            resize(rect.right, rect.bottom);
            return true;
        }

        virtual void destroy() {
        }

        virtual void resize(unsigned int w, unsigned int h) {
            if (m_width == w && m_height == h)
                return;

            base::CritScope cs(&m_CS);
            m_width = w;
            m_height = h;
            m_pixels = NULL;
        }

        virtual void render(wkeWebView webView) {
			base::CritScope cs(&m_CS);

            if (wkeIsDirty(webView)) {
                if (m_pixels == NULL)
                    createBitmap();

                if (m_pixels) {
                    wkePaint(webView, m_pixels, 0);

                    HDC hDC = GetDC(m_hView);
                    BitBlt(hDC, 0, 0, m_width, m_height, m_hDC, 0, 0, SRCCOPY);
                    ReleaseDC(m_hView, hDC);
                }
            }
        }

        virtual void renderDC(wkeWebView webView, HDC hDC) {
			base::CritScope cs(&m_CS);

            if (m_pixels == NULL)
                createBitmap();

            if(wkePaint && webView)
                wkePaint(webView, m_pixels, 0);

            BitBlt(hDC, 0, 0, m_width, m_height, m_hDC, 0, 0, SRCCOPY);
        }

        void createBitmap() {
            BITMAPINFO bi;
            memset(&bi, 0, sizeof(bi));
            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth = int(m_width);
            bi.bmiHeader.biHeight = -int(m_height);
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;

            HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_pixels, NULL, 0);

            SelectObject(m_hDC, hbmp);

            if (m_hBitmap)
                DeleteObject(m_hBitmap);

            m_hBitmap = hbmp;
        }

      private:
        HWND m_hView;
        HBITMAP m_hBitmap;
        HDC m_hDC;
        unsigned int m_width;
        unsigned int m_height;
        void *m_pixels;
        bool m_bInit;
        base::CriticalSection m_CS;
    };

    CWkeWnd::CWkeWnd(CPaintManagerUI *pParentPM) :
        m_pOwner(NULL),
        m_pWebView(NULL),
        m_bInit(false),
        m_iCursorInfoType(0),
        m_pParentPM(pParentPM),
        m_pTimer(NULL),
        m_pRender(NULL),
        m_pReadyCB(NULL),
        m_dwContext(0) {
        m_pRender = new RenderGDI();
    }

    CWkeWnd::~CWkeWnd() {
        delete m_pRender;
    }

    void CWkeWnd::RegisterDocumentReadyCallback(PFN_OnDocumentReadyNotify cb, DWORD context) {
        m_pReadyCB = cb;
        m_dwContext = context;
    }

    void CWkeWnd::SetUrl(const base::String &strUrl) {
        m_strUrl = strUrl;

        if (m_pWebView && m_strUrl.GetLength() > 0) {
            wkeLoadURL(m_pWebView, base::UnicodeToUtf8(m_strUrl).GetDataPointer());
        }
    }

    void CWkeWnd::SetBkImage(const base::String &strBkImage) {
        if (m_strBkImage != strBkImage) {
            m_strBkImage = strBkImage;
            m_diBkImage.Clear();
            m_diBkImage.sDrawString = m_strBkImage;
        }
    }

    void CWkeWnd::ShowDevTools() {
        if (m_pWebView && wkeShowDevtools) {
            std::wstring strInspectorPath = CPaintManagerUI::GetInstancePath().GetDataPointer();
            strInspectorPath += L"dev_tools\\inspector.html";

            if (_waccess(strInspectorPath.c_str(), 0) != 0)
                return;

            wkeShowDevtools(m_pWebView, strInspectorPath.c_str(), NULL, NULL);
        }
    }

    void CWkeWnd::GoForward() {
        if (m_pWebView && wkeCanGoForward && wkeGoForward) {
            if (wkeCanGoForward(m_pWebView)) {
                wkeGoForward(m_pWebView);
            }
        }
    }

    void CWkeWnd::GoBack() {
        if (m_pWebView && wkeCanGoBack && wkeGoBack) {
            if (wkeCanGoBack(m_pWebView)) {
                wkeGoBack(m_pWebView);
            }
        }
    }

    void CWkeWnd::Reload() {
        if (m_pWebView && wkeReload) {
            wkeReload(m_pWebView);
        }
    }

    static void jsValue2VARINAT(const jsExecState &es, const jsValue &retval, VARIANT *pvRet) {
        if (!pvRet) return;

        if (jsIsString(retval)) {
            V_VT(pvRet) = VT_BSTR;
            V_BSTR(pvRet) = SysAllocString(base::Utf8ToUnicode(jsToString(es, retval)).GetDataPointer());
        } else if (jsIsNumber(retval)) {
            V_VT(pvRet) = VT_I4;
            V_I4(pvRet) = jsToInt(es, retval);
        } else if (jsIsBoolean(retval)) {
            V_VT(pvRet) = VT_BOOL;
            V_BOOL(pvRet) = jsToBoolean(es, retval);
        }
    }

    VARIANT CWkeWnd::CallJavascriptFunction(const std::string &strFuncName, const std::vector<VARIANT> &vars) {
        VARIANT ret;

        if (!m_pWebView || !wkeGlobalExec || !jsCallGlobal || !jsGetGlobal) {
            V_VT(&ret) = VT_I4;
            V_I4(&ret) = 0;
            return ret;
        }

        //TimerMeter tm;
        jsExecState es = wkeGlobalExec(m_pWebView);
        jsValue *jsArgs = new jsValue[vars.size()];

        for (size_t i = 0; i < vars.size(); i++) {
            switch (vars[i].vt) {
                case VT_BSTR:
                    jsArgs[i] = jsString(es, base::UnicodeToUtf8(vars[i].bstrVal).GetDataPointer());
                    break;

                case VT_INT:
                case VT_I4:
                case VT_UINT: {
                        jsArgs[i] = jsInt(vars[i].intVal);
                        break;
                    }

                case VT_BOOL: {
                        jsArgs[i] = jsBoolean(vars[i].boolVal == VARIANT_TRUE);
                        break;
                    }

                case VT_R8:
                    jsArgs[i] = jsDouble(vars[i].dblVal);

                default:
                    break;
            }
        }

        jsValue jsret = jsCallGlobal(es, jsGetGlobal(es, strFuncName.c_str()), jsArgs, vars.size());
        delete[]jsArgs;
        jsValue2VARINAT(es, jsret, &ret);
        return ret;
    }

    void CWkeWnd::Init(CWkeUI *pOwner) {
        m_pOwner = pOwner;
        m_bInit = true;

        if (m_hWnd == NULL) {
            RECT rcPos = m_pOwner->GetPos();
            UINT uStyle = UI_WNDSTYLE_CHILD;

            Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
        }
    }

    LPCTSTR CWkeWnd::GetWindowClassName() const {
        return _T("WkeWebkitWindowClass");
    }

    void CWkeWnd::OnFinalMessage(HWND /*hWnd*/) {
        m_pOwner->Invalidate();

        m_pOwner->m_pWindow = NULL;
        delete this;
    }

    LRESULT CWkeWnd::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        KillTimer(m_hWnd, m_pTimer);
        m_pTimer = NULL;

        if (m_pWebView && wkeDestroyWebView) {
            wkeDestroyWebView(m_pWebView);
            m_pWebView = NULL;
        }

        bHandled = TRUE;
        return 0;
    }

    LRESULT CWkeWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        PAINTSTRUCT ps = { 0 };
        HDC hDcPaint = ::BeginPaint(m_hWnd, &ps);

        RECT r;
        GetClientRect(m_hWnd, &r);

        CRenderEngine::DrawImageInfo(hDcPaint, m_pParentPM, r, r, &m_diBkImage);

        m_pRender->renderDC(m_pWebView, ps.hdc);

        ::EndPaint(m_hWnd, &ps);
        return 0;
    }

    void CWkeWnd::OnDocumentReadyCallback(wkeWebView webView, void *param) {
        CWkeWnd *self = (CWkeWnd *)param;

        if (self) {
            if (self->m_pReadyCB) {
                self->m_pReadyCB(self->m_dwContext);
            }
        }
    }

    void CWkeWnd::OnPaintUpdatedCallback(wkeWebView webView, void *param, const HDC hdc, int x, int y, int cx, int cy) {
        CWkeWnd *self = (CWkeWnd *)param;

        HDC hDestDC = GetDC(self->m_hWnd);
        BitBlt(hDestDC, x, y, cx, cy, hdc, x, y, SRCCOPY);
    }


    LRESULT CWkeWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        DragAcceptFiles(GetHWND(), TRUE);

        //TimerMeter tm;
        m_pRender->init(m_hWnd);

        if (wkeCreateWebView) {
            m_pWebView = wkeCreateWebView();
        }

        if (m_pWebView) {
            wkeSetTransparent(m_pWebView, true);
            //wkeOnPaintUpdated(m_pWebView, OnPaintUpdatedCallback, this);
            wkeOnDocumentReady(m_pWebView, OnDocumentReadyCallback, this);

            wkeSetCookieEnabled(m_pWebView, true);
            //wchar_t szFolderPath[MAX_PATH + 2] = { 0 };
            //std::wstring path;

            //// GetTempPathW: The maximum possible return value is MAX_PATH+1 (261).
            //if (GetTempPathW(MAX_PATH + 2, szFolderPath) > 0) {
            //    PathAddBackslashW(szFolderPath);
            //    path = szFolderPath;
            //}
            //else {
            //    GetModuleFileNameW(NULL, szFolderPath, MAX_PATH);
            //    PathRemoveFileSpecW(szFolderPath);
            //    PathAddBackslashW(szFolderPath);
            //    path = szFolderPath;
            //}

            //path += L"PPXMINICEF_TEMP\\";

            //wkeSetCookieJarPath(m_pWebView, (path + L"cookie\\cookie.dat").c_str());
            //wkeSetLocalStorageFullPath(m_pWebView, (path + L"LocalStorage\\").c_str());

            if (wkeResize) {
                RECT rc;
                GetClientRect(m_hWnd, &rc);
                wkeResize(m_pWebView, rc.right - rc.left, rc.bottom - rc.top);
            }

            if (wkeSetHandle)
                wkeSetHandle(m_pWebView, m_hWnd);

            if (m_strUrl.GetLength() > 0)
                wkeLoadURL(m_pWebView, base::UnicodeToUtf8(m_strUrl.GetDataPointer()).GetDataPointer());

            m_pTimer = SetTimer(m_hWnd, WUM_REFRESH_TIMER, 50, NULL);
        }

        //TraceMsg(TEXT("CWebkitWnd::OnCreate: %d ms\n"), tm.Elapsed());
        return 0;
    }

    LRESULT CWkeWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        if (m_pWebView && wkeResize) {
            wkeResize(m_pWebView, LOWORD(lParam), HIWORD(lParam));
            m_pRender->resize(LOWORD(lParam), HIWORD(lParam));
        }

        return 0;
    }

    LRESULT CWkeWnd::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        switch (uMsg) {
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MOUSEMOVE: {
                    if(wkeGetCursorInfoType)
                        m_iCursorInfoType = wkeGetCursorInfoType(m_pWebView);

                    if (uMsg == WM_LBUTTONDOWN || uMsg == WM_MBUTTONDOWN || uMsg == WM_RBUTTONDOWN) {
                        SetFocus(m_hWnd);
                        SetCapture(m_hWnd);
                    } else if (uMsg == WM_LBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_RBUTTONUP) {
                        ReleaseCapture();
                    }

                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);

                    unsigned int flags = 0;

                    if (wParam & MK_CONTROL)
                        flags |= WKE_CONTROL;

                    if (wParam & MK_SHIFT)
                        flags |= WKE_SHIFT;

                    if (wParam & MK_LBUTTON)
                        flags |= WKE_LBUTTON;

                    if (wParam & MK_MBUTTON)
                        flags |= WKE_MBUTTON;

                    if (wParam & MK_RBUTTON)
                        flags |= WKE_RBUTTON;

                    if(wkeFireMouseEvent)
                        bHandled = wkeFireMouseEvent(m_pWebView, uMsg, x, y, flags);
                }

            default:
                break;
        }

        return 0;
    }

    LRESULT CWkeWnd::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(GetHWND(), &pt);

        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

        unsigned int flags = 0;

        if (wParam & MK_CONTROL)
            flags |= WKE_CONTROL;

        if (wParam & MK_SHIFT)
            flags |= WKE_SHIFT;

        if (wParam & MK_LBUTTON)
            flags |= WKE_LBUTTON;

        if (wParam & MK_MBUTTON)
            flags |= WKE_MBUTTON;

        if (wParam & MK_RBUTTON)
            flags |= WKE_RBUTTON;

        if(wkeFireMouseWheelEvent)
            bHandled = wkeFireMouseWheelEvent(m_pWebView, pt.x, pt.y, delta, flags);

        return 0;
    }

    LRESULT CWkeWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        unsigned int virtualKeyCode = wParam;
        unsigned int flags = 0;

        if (HIWORD(lParam) & KF_REPEAT)
            flags |= WKE_REPEAT;

        if (HIWORD(lParam) & KF_EXTENDED)
            flags |= WKE_EXTENDED;

        if(wkeFireKeyDownEvent)
            bHandled = wkeFireKeyDownEvent(m_pWebView, virtualKeyCode, flags, false);

        return 0;
    }

    LRESULT CWkeWnd::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        unsigned int virtualKeyCode = wParam;
        unsigned int flags = 0;

        if (HIWORD(lParam) & KF_REPEAT)
            flags |= WKE_REPEAT;

        if (HIWORD(lParam) & KF_EXTENDED)
            flags |= WKE_EXTENDED;

        if(wkeFireKeyUpEvent)
            bHandled = wkeFireKeyUpEvent(m_pWebView, virtualKeyCode, flags, false);

        return 0;
    }

    LRESULT CWkeWnd::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        unsigned int charCode = wParam;
        unsigned int flags = 0;

        if (HIWORD(lParam) & KF_REPEAT)
            flags |= WKE_REPEAT;

        if (HIWORD(lParam) & KF_EXTENDED)
            flags |= WKE_EXTENDED;

        if(wkeFireKeyPressEvent)
            bHandled = wkeFireKeyPressEvent(m_pWebView, charCode, flags, false);

        return 0;
    }

    LRESULT CWkeWnd::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        if (wParam == WUM_REFRESH_TIMER) {
            if (!IsIconic(m_hWnd)) {
                if (wkeIsDirty && m_pWebView) {
                    if (wkeIsDirty(m_pWebView)) {
                        m_pRender->render(m_pWebView);
                    }
                }
            }
        }

        bHandled = true;
        return 0;
    }

    LRESULT CWkeWnd::OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        if (m_pWebView && wkeGetCaretRect) {
            wkeRect caret = wkeGetCaretRect(m_pWebView);

            CANDIDATEFORM form;
            form.dwIndex = 0;
            form.dwStyle = CFS_EXCLUDE;
            form.ptCurrentPos.x = caret.x;
            form.ptCurrentPos.y = caret.y + caret.h;
            form.rcArea.top = caret.y;
            form.rcArea.bottom = caret.y + caret.h;
            form.rcArea.left = caret.x;
            form.rcArea.right = caret.x + caret.w;

            HIMC hIMC = ImmGetContext(m_hWnd);
            ImmSetCandidateWindow(hIMC, &form);
            ImmReleaseContext(m_hWnd, hIMC);
        }

        return 0;
    }

    LRESULT CWkeWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        if(m_pWebView && wkeSetFocus)
            wkeSetFocus(m_pWebView);

        return 0;
    }

    LRESULT CWkeWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        if(m_pWebView && wkeKillFocus)
            wkeKillFocus(m_pWebView);

        return 0;
    }

    LRESULT CWkeWnd::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        if (pt.x != -1 && pt.y != -1)
            ScreenToClient(m_hWnd, &pt);

        unsigned int flags = 0;

        if (wParam & MK_CONTROL)
            flags |= WKE_CONTROL;

        if (wParam & MK_SHIFT)
            flags |= WKE_SHIFT;

        if (wParam & MK_LBUTTON)
            flags |= WKE_LBUTTON;

        if (wParam & MK_MBUTTON)
            flags |= WKE_MBUTTON;

        if (wParam & MK_RBUTTON)
            flags |= WKE_RBUTTON;

        if(wkeFireContextMenuEvent)
            bHandled = wkeFireContextMenuEvent(m_pWebView, pt.x, pt.y, flags);

        return 0;
    }

    bool CWkeWnd::SetCursorInfoTypeByCache(HWND hWnd) {
        RECT rc;
        ::GetClientRect(hWnd, &rc);

        POINT pt;
        ::GetCursorPos(&pt);
        ::ScreenToClient(hWnd, &pt);

        if (!::PtInRect(&rc, pt))
            return false;

        HCURSOR hCur = NULL;

        switch (m_iCursorInfoType) {
            case WkeCursorInfoPointer:
                hCur = ::LoadCursor(NULL, IDC_ARROW);
                break;

            case WkeCursorInfoIBeam:
                hCur = ::LoadCursor(NULL, IDC_IBEAM);
                break;

            case WkeCursorInfoHand:
                hCur = ::LoadCursor(NULL, IDC_HAND);
                break;

            case WkeCursorInfoWait:
                hCur = ::LoadCursor(NULL, IDC_WAIT);
                break;

            case WkeCursorInfoHelp:
                hCur = ::LoadCursor(NULL, IDC_HELP);
                break;

            case WkeCursorInfoEastResize:
                hCur = ::LoadCursor(NULL, IDC_SIZEWE);
                break;

            case WkeCursorInfoNorthResize:
                hCur = ::LoadCursor(NULL, IDC_SIZENS);
                break;

            case WkeCursorInfoSouthWestResize:
            case WkeCursorInfoNorthEastResize:
                hCur = ::LoadCursor(NULL, IDC_SIZENESW);
                break;

            case WkeCursorInfoSouthResize:
            case WkeCursorInfoNorthSouthResize:
                hCur = ::LoadCursor(NULL, IDC_SIZENS);
                break;

            case WkeCursorInfoNorthWestResize:
            case WkeCursorInfoSouthEastResize:
                hCur = ::LoadCursor(NULL, IDC_SIZENWSE);
                break;

            case WkeCursorInfoWestResize:
            case WkeCursorInfoEastWestResize:
                hCur = ::LoadCursor(NULL, IDC_SIZEWE);
                break;

            case WkeCursorInfoNorthEastSouthWestResize:
            case WkeCursorInfoNorthWestSouthEastResize:
                hCur = ::LoadCursor(NULL, IDC_SIZEALL);
                break;

            default:
                hCur = ::LoadCursor(NULL, IDC_ARROW);
                break;
        }

        if (hCur) {
            ::SetCursor(hCur);
            return true;
        }

        return false;
    }

    LRESULT CWkeWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
        LRESULT lRes = 0;
        BOOL bHandled = TRUE;

        switch (uMsg) {
            case WM_PAINT:
                lRes = OnPaint(uMsg, wParam, lParam, bHandled);
                break;

            case WM_CREATE:
                lRes = OnCreate(uMsg, wParam, lParam, bHandled);
                break;

            case WM_DESTROY:
                lRes = OnDestroy(uMsg, wParam, lParam, bHandled);
                break;

            case WM_MOUSEWHEEL:
                lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled);
                break;

            case WM_SIZE:
                lRes = OnSize(uMsg, wParam, lParam, bHandled);
                break;

            case WM_CHAR:
                lRes = OnChar(uMsg, wParam, lParam, bHandled);
                break;

            case WM_KEYDOWN:
                lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
                break;

            case WM_KEYUP:
                lRes = OnKeyUp(uMsg, wParam, lParam, bHandled);
                break;

            case WM_KILLFOCUS:
                lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
                break;

            case WM_SETFOCUS:
                lRes = OnSetFocus(uMsg, wParam, lParam, bHandled);
                break;

            case WM_IME_STARTCOMPOSITION:
                lRes = OnImeStartComposition(uMsg, wParam, lParam, bHandled);
                break;

            case WM_CONTEXTMENU:
                lRes = OnContextMenu(uMsg, wParam, lParam, bHandled);
                break;

            case WM_TIMER:
                lRes = OnTimer(uMsg, wParam, lParam, bHandled);
                break;

            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_MOUSEMOVE:
            case WM_MOUSELEAVE:
                lRes = OnMouseEvent(uMsg, wParam, lParam, bHandled);
                break;

            case WM_SETCURSOR: {
                    SetCursorInfoTypeByCache(m_hWnd);
                    lRes = 0;
                    bHandled = true;
                    break;
                }

            default:
                bHandled = FALSE;
                break;
        }

        if (bHandled) return lRes;

        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    CWkeUI::PFN_OnJSNotify CWkeUI::m_pfnOnJSNotify = NULL;
    DWORD CWkeUI::m_dwContext = 0;



    static jsValue JS_CALL JSNotifyCppFunc_1(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);

        vars.push_back(v);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }

    static jsValue JS_CALL JSNotifyCppFunc_2(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);
        vars.push_back(v);

        VARIANT v1;
        jsValue2VARINAT(es, jsArg(es, 1), &v1);
        vars.push_back(v1);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }

    static jsValue JS_CALL JSNotifyCppFunc_3(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);
        vars.push_back(v);

        VARIANT v1;
        jsValue2VARINAT(es, jsArg(es, 1), &v1);
        vars.push_back(v1);

        VARIANT v2;
        jsValue2VARINAT(es, jsArg(es, 2), &v2);
        vars.push_back(v2);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }


    static jsValue JS_CALL JSNotifyCppFunc_4(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);
        vars.push_back(v);

        VARIANT v1;
        jsValue2VARINAT(es, jsArg(es, 1), &v1);
        vars.push_back(v1);

        VARIANT v2;
        jsValue2VARINAT(es, jsArg(es, 2), &v2);
        vars.push_back(v2);

        VARIANT v3;
        jsValue2VARINAT(es, jsArg(es, 3), &v3);
        vars.push_back(v3);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }


    static jsValue JS_CALL JSNotifyCppFunc_5(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);
        vars.push_back(v);

        VARIANT v1;
        jsValue2VARINAT(es, jsArg(es, 1), &v1);
        vars.push_back(v1);

        VARIANT v2;
        jsValue2VARINAT(es, jsArg(es, 2), &v2);
        vars.push_back(v2);

        VARIANT v3;
        jsValue2VARINAT(es, jsArg(es, 3), &v3);
        vars.push_back(v3);

        VARIANT v4;
        jsValue2VARINAT(es, jsArg(es, 4), &v4);
        vars.push_back(v4);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }


    static jsValue JS_CALL JSNotifyCppFunc_6(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);
        vars.push_back(v);

        VARIANT v1;
        jsValue2VARINAT(es, jsArg(es, 1), &v1);
        vars.push_back(v1);

        VARIANT v2;
        jsValue2VARINAT(es, jsArg(es, 2), &v2);
        vars.push_back(v2);

        VARIANT v3;
        jsValue2VARINAT(es, jsArg(es, 3), &v3);
        vars.push_back(v3);

        VARIANT v4;
        jsValue2VARINAT(es, jsArg(es, 4), &v4);
        vars.push_back(v4);

        VARIANT v5;
        jsValue2VARINAT(es, jsArg(es, 5), &v5);
        vars.push_back(v5);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }


    static jsValue JS_CALL JSNotifyCppFunc_7(jsExecState es) {
        std::vector<VARIANT> vars;
        VARIANT v;
        jsValue2VARINAT(es, jsArg(es, 0), &v);
        vars.push_back(v);

        VARIANT v1;
        jsValue2VARINAT(es, jsArg(es, 1), &v1);
        vars.push_back(v1);

        VARIANT v2;
        jsValue2VARINAT(es, jsArg(es, 2), &v2);
        vars.push_back(v2);

        VARIANT v3;
        jsValue2VARINAT(es, jsArg(es, 3), &v3);
        vars.push_back(v3);

        VARIANT v4;
        jsValue2VARINAT(es, jsArg(es, 4), &v4);
        vars.push_back(v4);

        VARIANT v5;
        jsValue2VARINAT(es, jsArg(es, 5), &v5);
        vars.push_back(v5);

        VARIANT v6;
        jsValue2VARINAT(es, jsArg(es, 6), &v6);
        vars.push_back(v6);

        std::string ret;

        if (CWkeUI::m_pfnOnJSNotify) {
            ret = CWkeUI::m_pfnOnJSNotify(vars, CWkeUI::m_dwContext);
        }

        return jsString(es, ret.c_str());
    }

    IMPLEMENT_DUICONTROL(CWkeUI)

    CWkeUI::CWkeUI(void) : m_pWindow(NULL) {
    }

    CWkeUI::~CWkeUI(void) {
    }

    void CWkeUI::WkeInit() {
        wchar_t szExePath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, szExePath, MAX_PATH);
        PathRemoveFileSpecW(szExePath);
        PathAddBackslashW(szExePath);

        std::wstring strDllPath = szExePath;
        strDllPath += L"node.dll";

        if (_waccess(strDllPath.c_str(), 0) != 0) {
            TraceMsg(TEXT("Duilib: Î´ÕÒµ½node.dll"));
            assert(false);
            return;
        }

        wkeSetWkeDllPath(strDllPath.c_str());
        wkeInitialize();

        wkeSettings settings;
        memset(&settings, 0, sizeof(settings));

        if(wkeConfigure)
            wkeConfigure(&settings);

        if (jsBindFunction) {
            jsBindFunction("JSNotifyCppFunc_1", JSNotifyCppFunc_1, 1);
            jsBindFunction("JSNotifyCppFunc_2", JSNotifyCppFunc_2, 2);
            jsBindFunction("JSNotifyCppFunc_3", JSNotifyCppFunc_3, 3);
            jsBindFunction("JSNotifyCppFunc_4", JSNotifyCppFunc_4, 4);
            jsBindFunction("JSNotifyCppFunc_5", JSNotifyCppFunc_5, 5);
            jsBindFunction("JSNotifyCppFunc_6", JSNotifyCppFunc_6, 6);
            jsBindFunction("JSNotifyCppFunc_7", JSNotifyCppFunc_7, 7);
        }
    }

    void CWkeUI::WkeShutdown() {
        if(wkeFinalize)
            wkeFinalize();
    }

    void CWkeUI::RegisterJSNotifyCallback(PFN_OnJSNotify cb, DWORD context) {
        m_pfnOnJSNotify = cb;
        m_dwContext = context;
    }

    void CWkeUI::RegisterDocumentReadyCallback(PFN_OnDocumentReadyNotify cb, DWORD context) {
        if (m_pWindow) {
            m_pWindow->RegisterDocumentReadyCallback(cb, context);
        }
    }

    void CWkeUI::SetVisible(bool bVisible) {
        m_pWindow->ShowWindow(bVisible);
        CControlUI::SetVisible(bVisible);
        SetInternVisible(bVisible);
    }

    void CWkeUI::DoInit() {
        m_pWindow = new CWkeWnd(m_pManager);

        if (m_pWindow) {
            m_pWindow->SetUrl(m_strUrl);
            m_pWindow->SetBkImage(m_strBkImage);
            m_pWindow->Init(this);
            m_pWindow->ShowWindow();
        }
    }

    LPCTSTR CWkeUI::GetClass() const {
        return DUI_CTR_WEBKIT;
    }

    LPVOID CWkeUI::GetInterface(LPCTSTR pstrName) {
        if (_tcscmp(pstrName, DUI_CTR_WEBKIT) == 0) return static_cast<CWkeUI *>(this);

        return CControlUI::GetInterface(pstrName);
    }

    void CWkeUI::DoEvent(TEventUI &event) {
        if (event.Type == UIEVENT_SETCURSOR) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE((DWORD)IDC_ARROW)));
            return;
        }

        CControlUI::DoEvent(event);
    }

    void CWkeUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
        if (_tcscmp(pstrName, _T("url")) == 0) {
            SetURL(pstrValue);
        } else if (_tcscmp(pstrName, _T("bkimage")) == 0) {
            SetBkImage(pstrValue);
        } else {
            CControlUI::SetAttribute(pstrName, pstrValue);
        }
    }

    void CWkeUI::SetPos(RECT rc, bool bNeedInvalidate /*= true*/) {
        CControlUI::SetPos(rc, bNeedInvalidate);
        ::SetWindowPos(m_pWindow->GetHWND(), NULL, rc.left, rc.top, rc.right - rc.left,
                       rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void CWkeUI::SetURL(const base::String &strValue) {
        m_strUrl = strValue;

        if(m_pWindow)
            m_pWindow->SetUrl(strValue);
    }

    void CWkeUI::SetBkImage(const base::String &strValue) {
        m_strBkImage = strValue;

        if(m_pWindow)
            m_pWindow->SetBkImage(strValue);
    }

    VARIANT CWkeUI::CallJavascriptFunction(const std::string &strFuncName, const std::vector<VARIANT> &vars) {
        if (m_pWindow) {
            return m_pWindow->CallJavascriptFunction(strFuncName, vars);
        }

        VARIANT ret;
        V_VT(&ret) = VT_I4;
        V_I4(&ret) = 0;
        return ret;
    }

    void CWkeUI::GoBack() {
        if (m_pWindow) {
            m_pWindow->GoBack();
        }
    }

    void CWkeUI::GoForward() {
        if (m_pWindow) {
            m_pWindow->GoForward();
        }
    }

    void CWkeUI::Reload() {
        if (m_pWindow) {
            m_pWindow->Reload();
        }
    }

    void CWkeUI::ShowDevTools() {
        if (m_pWindow) {
            m_pWindow->ShowDevTools();
        }
    }

}