#ifdef UILIB_WITH_CEF
#include "StdAfx.h"
#include "UICef.h"
#include <fstream>
#include <cstring>
#include "ppxbase/stringencode.h"
#include "ppxbase/criticalsection.h"
#include "ppxbase/random.h"
#include "Internal/Cef/RequestContextHandler.h"
#include "Internal/Cef/CefHandler.h"
#include "Internal/Cef/CefUtil.h"
#include "include/base/cef_build.h"
#include "Internal/Cef/CefDevTools.h"
#include "Utils/Task.h"


namespace DuiLib {

    class CCefUI::CCefUIImpl : public Internal::ClientHandlerOsr::OsrDelegate {
      public:
        CCefUIImpl(CCefUI *parent) :
            m_pParent(parent)
            , m_pBuffer(NULL)
            , m_hBitmap(NULL)
            , m_hMemoryDC(NULL)
            , m_pDevToolsWnd(NULL)
            , m_iMemoryBitmapWidth(0)
            , m_iMemoryBitmapHeight(0)
            , last_mouse_pos_()
            , current_mouse_pos_()
            , mouse_pos_()
            , mouse_rotation_(false)
            , mouse_tracking_(false)
            , last_click_x_(0)
            , last_click_y_(0)
            , last_click_button_(MBT_LEFT)
            , last_click_count_(0)
            , last_click_time_(0)
            , last_mouse_down_on_view_(false)
            , m_iFPS(60)
            , m_bBkTransparent(false)
            , m_dwCefBkColor(0xffffffff) {
            m_hMemoryDC = CreateCompatibleDC(NULL);
            ppx::base::Random random;
            m_iRandomID = random.Rand(0xFFFFFFFF);
        }

        ~CCefUIImpl() {
            if (m_pDevToolsWnd) {
                TCHAR szName[MAX_PATH];
                StringCchPrintf(szName, MAX_PATH, TEXT("Dev Tools[%lu]"), m_iRandomID);
                HWND h = FindWindow(TEXT("CefDevToolsWnd891$322@"), szName);
                if (h) {
                    m_pDevToolsWnd->Close();
                    return;
                }
                m_pDevToolsWnd = NULL;
            }
            m_ClientHandler->DetachDelegate();
            CloseBrowser();

            if (m_hMemoryDC) {
                DeleteDC(m_hMemoryDC);
                m_hMemoryDC = NULL;
            }
        }

      public:
        void CreateBrowser() {
            DCHECK(!m_browser);
            CefBrowserSettings browser_settings;
            browser_settings.windowless_frame_rate = m_iFPS;

            if (!m_bBkTransparent) {
                browser_settings.background_color = CefColorSetARGB(255, GetBValue(m_dwCefBkColor), GetGValue(m_dwCefBkColor), GetRValue(m_dwCefBkColor));
            }

            CefRequestContextSettings context_settings;
            CefRefPtr<CefRequestContext> request_context = CefRequestContext::CreateContext(
                        CefRequestContext::GetGlobalContext(),
                        new Internal::RequestContextHandler);

            CefWindowInfo window_info;
            window_info.SetAsWindowless(m_pParent->m_pManager->GetPaintWindow(), m_bBkTransparent);
            if (GetWindowLongPtr(m_pParent->m_pManager->GetPaintWindow(), GWL_EXSTYLE) & WS_EX_NOACTIVATE) {
                window_info.ex_style |= WS_EX_NOACTIVATE;
            }

            m_ClientHandler = new Internal::ClientHandlerOsr(this);
            m_ClientHandler->SetAllowProtocols(m_vAllowProtocols);

            m_strInitUrl = m_pParent->GetUrl();

            CefBrowserHost::CreateBrowser(window_info, m_ClientHandler,
                                          ppx::base::UnicodeToUtf8(m_strInitUrl.GetData()), browser_settings, request_context);
        }

        void CloseBrowser() {
            if (m_browser) {
                m_browser->GetHost()->CloseBrowser(false);
            }
        }

        void SetUrl(const CDuiString &url) {
            if (m_browser) {
                if (m_browser->GetMainFrame()) {
                    m_browser->GetMainFrame()->LoadURL(ppx::base::UnicodeToUtf8(m_pParent->GetUrl().GetData()).c_str());
                }
            }
        }

        void GoBack() {
            if (m_browser)
                m_browser->GoBack();
        }

        void GoForward() {
            if (m_browser)
                m_browser->GoForward();
        }

        void Reload() {
            if (m_browser)
                m_browser->Reload();
        }

        void ShowDevTools() {
            if (!m_browser)
                return;
            TCHAR szName[MAX_PATH];
            StringCchPrintf(szName, MAX_PATH, TEXT("Dev Tools[%lu]"), m_iRandomID);

            HWND h = FindWindow(TEXT("CefDevToolsWnd891$322@"), szName);
            if (h) {
                SetForegroundWindow(h);
                return;
            }

            m_pDevToolsWnd = new Internal::CefDevToolsWnd(m_browser, m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f);
            m_pDevToolsWnd->Create(NULL, szName, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT,
                                   CW_USEDEFAULT);
            m_pDevToolsWnd->ShowWindow();
        }

        void CloseDevTools() {
            if (m_pDevToolsWnd) {
                m_pDevToolsWnd->Close();
                m_pDevToolsWnd = NULL; // delete in DevToolsWnd internal.
            }
        }

        bool CallJavascriptFunction(const std::string &strFuncName, const std::vector<CLiteVariant> &args) {
            bool ret = false;
            if (m_browser) {
                CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create(Internal::BROWSER_2_RENDER_CPP_CALL_JS_MSG);
                CefRefPtr<CefListValue> arg_list = message->GetArgumentList();
                arg_list->SetString(0, strFuncName);

                int cefListIndex = 1;
                for (auto item : args) {
                    CLiteVariant::DataType dt = item.GetType();
                    if (dt == CLiteVariant::DataType::DT_INT) {
                        arg_list->SetInt(cefListIndex, item.GetInt());
                    } else if (dt == CLiteVariant::DataType::DT_DOUBLE) {
                        arg_list->SetDouble(cefListIndex, item.GetDouble());
                    } else if (dt == CLiteVariant::DataType::DT_STRING) {
                        arg_list->SetString(cefListIndex, item.GetString());
                    }

                    cefListIndex++;
                }
                ret = m_browser->SendProcessMessage(PID_RENDERER, message);
            }
            return true;
        }

        // CefContextMenuHandler methods
        void OnBeforeContextMenu(CefRefPtr<CefMenuModel> model) OVERRIDE {
            model->Clear();
        }

        //////////////////////////////////////////////////////////////////////////
        // ClientHandlerOsr::OsrDelegate methods.
        void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE {
            m_browser = browser;
            if (m_pParent && m_browser) {
                if (m_pParent->GetUrl() != m_strInitUrl) {
                    if (m_browser->GetMainFrame()) {
                        m_browser->GetMainFrame()->LoadURL(ppx::base::UnicodeToUtf8(m_pParent->GetUrl().GetData()).c_str());
                    }
                }
            }
        }

        void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE {
            m_browser = nullptr;
        }

        bool GetRootScreenRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE {
            CEF_REQUIRE_UI_THREAD();
            return false;
        }
#if CEFVER == 3626
        void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE;
#else
        bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) OVERRIDE {
            CEF_REQUIRE_UI_THREAD();
            DCHECK(m_pParent);
            DCHECK(m_pParent->m_pManager);

            RECT pos = m_pParent->GetPos();
            int width = pos.right - pos.left;
            int height = pos.bottom - pos.top;

            float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;

            rect.x = pos.left;
            rect.y = pos.top;
            rect.width = Internal::DeviceToLogical(width, scale_factor);
            if (rect.width == 0)
                rect.width = 1;

            rect.height = Internal::DeviceToLogical(height, scale_factor);
            if (rect.height == 0)
                rect.height = 1;
            return true;
        }
#endif
        bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int &screenX, int &screenY) OVERRIDE {
            CEF_REQUIRE_UI_THREAD();
            DCHECK(m_pParent);
            DCHECK(m_pParent->m_pManager);

            float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;

            // Convert the point from view coordinates to actual screen coordinates.
            POINT screen_pt = {
                Internal::LogicalToDevice(viewX, scale_factor),
                Internal::LogicalToDevice(viewY, scale_factor)
            };
            ClientToScreen(m_pParent->m_pManager->GetPaintWindow(), &screen_pt);
            screenX = screen_pt.x;
            screenY = screen_pt.y;
            return true;
        }

        bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo &screen_info) OVERRIDE {
            CEF_REQUIRE_UI_THREAD();
            DCHECK(m_pParent);
            DCHECK(m_pParent->m_pManager);

            CefRect view_rect;
            GetViewRect(browser, view_rect);

            screen_info.device_scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;
            screen_info.rect = view_rect;
            screen_info.available_rect = view_rect;
            return true;
        }

        void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) OVERRIDE {
        }

        void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect &rect) OVERRIDE {
        }

        void OnPaint(CefRefPtr<CefBrowser> browser, CefRenderHandler::PaintElementType type,
                     const CefRenderHandler::RectList &dirtyRects, const void *buffer, int width, int height) OVERRIDE {
            m_csBuf.Enter();
            if (width != m_iMemoryBitmapWidth || height != m_iMemoryBitmapHeight) {
                BITMAPINFO bi;
                memset(&bi, 0, sizeof(bi));
                bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bi.bmiHeader.biWidth = int(width);
                bi.bmiHeader.biHeight = -int(height);
                bi.bmiHeader.biPlanes = 1;
                bi.bmiHeader.biBitCount = 32;
                bi.bmiHeader.biCompression = BI_RGB;

                HBITMAP hbmp = ::CreateDIBSection(0, &bi, DIB_RGB_COLORS, &m_pBuffer, NULL, 0);
                ::SelectObject(m_hMemoryDC, hbmp);
                if (m_hBitmap)
                    ::DeleteObject(m_hBitmap);

                m_hBitmap = hbmp;

                m_iMemoryBitmapWidth = width;
                m_iMemoryBitmapHeight = height;
            }

            memcpy(m_pBuffer, buffer, width *height * 4);
            m_csBuf.Leave();

            m_pParent->Invalidate();
        }

        void DoPaint(HDC hdc) {
            ppx::base::CritScope cs(&m_csBuf);
            if (m_pParent) {
                RECT rect = m_pParent->GetPos();
                if (m_iMemoryBitmapHeight > 0 && m_iMemoryBitmapWidth > 0) {
                    BitBlt(hdc, rect.left, rect.top, m_iMemoryBitmapWidth, m_iMemoryBitmapHeight, m_hMemoryDC, 0, 0, SRCCOPY);
                }
            }
        }

        void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser,
                                CefRenderHandler::PaintElementType type, const CefRenderHandler::RectList &dirtyRects, void *share_handle) OVERRIDE {

        }

        void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CefRenderHandler::CursorType type, const CefCursorInfo &custom_cursor_info) OVERRIDE {
            CEF_REQUIRE_UI_THREAD();
            HWND hwnd = m_pParent->m_pManager->GetPaintWindow();
            if (!hwnd || !::IsWindow(hwnd))
                return;

            // Change the plugin window's cursor.
            SetClassLongPtr(hwnd, GCLP_HCURSOR, static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
            ::SetCursor(cursor);
        }

        bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y) OVERRIDE {
            CEF_REQUIRE_UI_THREAD();
            // Cancel the drag. The dragging implementation requires ATL support.
            return false;
        }

        void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
                              CefRenderHandler::DragOperation operation) OVERRIDE {
        }
#if CEFVER == 3626
        void OnImeCompositionRangeChanged(
            CefRefPtr<CefBrowser> browser,
            const CefRange &selection_range,
            const CefRenderHandler::RectList &character_bounds) OVERRIDE;
#endif
        void UpdateAccessibilityTree(CefRefPtr<CefValue> value) OVERRIDE {
        }

        void OnBrowserClosing(CefRefPtr<CefBrowser> browser) OVERRIDE {
        }

        void OnSetAddress(const std::string &url) OVERRIDE {
        }

        void OnSetTitle(const std::string &title) OVERRIDE {
        }

        void OnSetFullscreen(bool fullscreen) OVERRIDE {
        }

        void OnAutoResize(const CefSize &new_size) OVERRIDE {
        }

        void OnSetLoadingState(bool isLoading, bool canGoBack, bool canGoForward) OVERRIDE {
        }

        void OnSetDraggableRegions(const std::vector<CefDraggableRegion> &regions) OVERRIDE {
        }

        bool OnTooltip(CefRefPtr<CefBrowser> browser, CefString &text) OVERRIDE {
            if (m_pParent && m_pParent->m_pManager) {
                if (text.length() == 0) {
                    m_pParent->m_pManager->HideToolTip();
                } else {
                    m_pParent->m_pManager->ShowToolTip(text.c_str(), mouse_pos_);
                }
            }

            return true;
        }

        void OnJSNotify(const CefRefPtr<CefListValue> &value_list) OVERRIDE {
            if (value_list->GetSize() < 2)
                return;

            std::string strBusinessName = value_list->GetString(1);
            std::vector<CLiteVariant> liteVars;
            for (size_t i = 2; i < value_list->GetSize(); i++) {
                CLiteVariant liteTmp;
                CefValueType type = value_list->GetType(i);
                switch (type) {
                    case VTYPE_BOOL: {
                        liteTmp.SetType(CLiteVariant::DataType::DT_INT);
                        liteTmp.SetInt(value_list->GetBool(i) ? 1 : 0);
                    }
                    break;
                    case VTYPE_DOUBLE: {
                        liteTmp.SetType(CLiteVariant::DataType::DT_INT);
                        liteTmp.SetDouble(value_list->GetDouble(i));
                    }
                    break;
                    case VTYPE_INT: {
                        liteTmp.SetType(CLiteVariant::DataType::DT_INT);
                        liteTmp.SetInt(value_list->GetInt(i));
                    }
                    break;
                    case VTYPE_STRING: {
                        liteTmp.SetType(CLiteVariant::DataType::DT_STRING);
                        liteTmp.SetString(value_list->GetString(i).ToString());
                    }
                    break;
                    default:
                        break;
                }
                liteVars.push_back(liteTmp);
            }

            if (m_pParent && m_pParent->m_JSCB) {
                m_pParent->m_JSCB(strBusinessName, liteVars);
            }
        }

        void OnResourceResponse(const std::string url, int rsp_status) OVERRIDE {
            if (m_pParent && m_pParent->m_ResourceRspCB) {
                m_pParent->m_ResourceRspCB(url, rsp_status);
            }
        }

        bool OnBeforePopup(const std::string &target_url) OVERRIDE {
            return true;
        }

        void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
            DCHECK(m_pParent);
            DCHECK(m_pParent->m_pManager);
            float scale_factor = m_pParent->m_pManager->GetDPIObj()->GetScale() / 100.f;
            HWND hwnd = m_pParent->m_pManager->GetPaintWindow();
            RECT pos = m_pParent->GetPos();

            CefRefPtr<CefBrowserHost> browser_host;
            if (m_browser)
                browser_host = m_browser->GetHost();

            LONG currentTime = 0;
            bool cancelPreviousClick = false;

            if (message == UIEVENT_BUTTONDOWN || message == UIEVENT_RBUTTONDOWN ||
                    message == UIEVENT_MBUTTONDOWN || message == UIEVENT_MOUSEMOVE ||
                    message == UIEVENT_MOUSELEAVE) {
                currentTime = GetMessageTime();
                int x = GET_X_LPARAM(lParam) - pos.left;
                int y = GET_Y_LPARAM(lParam) - pos.top;
                cancelPreviousClick =
                    (abs(last_click_x_ - x) > (GetSystemMetrics(SM_CXDOUBLECLK) / 2)) ||
                    (abs(last_click_y_ - y) > (GetSystemMetrics(SM_CYDOUBLECLK) / 2)) ||
                    ((currentTime - last_click_time_) > GetDoubleClickTime());
                if (cancelPreviousClick &&
                        (message == UIEVENT_MOUSEMOVE || message == UIEVENT_MOUSELEAVE)) {
                    last_click_count_ = 0;
                    last_click_x_ = 0;
                    last_click_y_ = 0;
                    last_click_time_ = 0;
                }
            }

            switch (message) {
                case UIEVENT_BUTTONDOWN:
                case UIEVENT_RBUTTONDOWN:
                case UIEVENT_MBUTTONDOWN: {
                    ::SetCapture(hwnd);
                    ::SetFocus(hwnd);
                    int x = GET_X_LPARAM(lParam) - pos.left;
                    int y = GET_Y_LPARAM(lParam) - pos.top;
                    if (wParam & MK_SHIFT) {
                        // Start rotation effect.
                        last_mouse_pos_.x = current_mouse_pos_.x = x;
                        last_mouse_pos_.y = current_mouse_pos_.y = y;
                        mouse_rotation_ = true;
                    } else {
                        CefBrowserHost::MouseButtonType btnType =
                            (message == UIEVENT_BUTTONDOWN
                             ? MBT_LEFT
                             : (message == UIEVENT_RBUTTONDOWN ? MBT_RIGHT : MBT_MIDDLE));
                        if (!cancelPreviousClick && (btnType == last_click_button_)) {
                            ++last_click_count_;
                        } else {
                            last_click_count_ = 1;
                            last_click_x_ = x;
                            last_click_y_ = y;
                        }
                        last_click_time_ = currentTime;
                        last_click_button_ = btnType;

                        if (browser_host) {
                            CefMouseEvent mouse_event;
                            mouse_event.x = x;
                            mouse_event.y = y;
                            //last_mouse_down_on_view_ = !IsOverPopupWidget(x, y);
                            //ApplyPopupOffset(mouse_event.x, mouse_event.y);
                            Internal::DeviceToLogical(mouse_event, scale_factor);
                            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                            browser_host->SendMouseClickEvent(mouse_event, btnType, false,
                                                              last_click_count_);
                        }
                    }
                }
                break;

                case UIEVENT_BUTTONUP:
                case UIEVENT_RBUTTONUP:
                case UIEVENT_MBUTTONUP: {
                    if (GetCapture() == hwnd)
                        ReleaseCapture();
                    if (mouse_rotation_) {
                        // End rotation effect.
                        mouse_rotation_ = false;
                    } else {
                        int x = GET_X_LPARAM(lParam) - pos.left;
                        int y = GET_Y_LPARAM(lParam) - pos.top;
                        CefBrowserHost::MouseButtonType btnType =
                            (message == UIEVENT_BUTTONUP
                             ? MBT_LEFT
                             : (message == UIEVENT_RBUTTONUP ? MBT_RIGHT : MBT_MIDDLE));
                        if (browser_host) {
                            CefMouseEvent mouse_event;
                            mouse_event.x = x;
                            mouse_event.y = y;
                            //if (last_mouse_down_on_view_ && IsOverPopupWidget(x, y) &&
                            //	(GetPopupXOffset() || GetPopupYOffset())) {
                            //	break;
                            //}
                            //ApplyPopupOffset(mouse_event.x, mouse_event.y);
                            Internal::DeviceToLogical(mouse_event, scale_factor);
                            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                            browser_host->SendMouseClickEvent(mouse_event, btnType, true,
                                                              last_click_count_);
                        }
                    }
                    break;
                }
                case UIEVENT_MOUSEMOVE: {
                    int x = GET_X_LPARAM(lParam) - pos.left;
                    int y = GET_Y_LPARAM(lParam) - pos.top;

                    mouse_pos_.x = x;
                    mouse_pos_.y = y;

                    if (mouse_rotation_) {
                        // Apply rotation effect.
                        current_mouse_pos_.x = x;
                        current_mouse_pos_.y = y;
                        last_mouse_pos_.x = current_mouse_pos_.x;
                        last_mouse_pos_.y = current_mouse_pos_.y;
                    } else {
                        if (!mouse_tracking_) {
                            // Start tracking mouse leave. Required for the WM_MOUSELEAVE event to be generated.
                            TRACKMOUSEEVENT tme;
                            tme.cbSize = sizeof(TRACKMOUSEEVENT);
                            tme.dwFlags = TME_LEAVE;
                            tme.hwndTrack = hwnd;
                            TrackMouseEvent(&tme);
                            mouse_tracking_ = true;
                        }

                        if (browser_host) {
                            CefMouseEvent mouse_event;
                            mouse_event.x = x;
                            mouse_event.y = y;
                            //ApplyPopupOffset(mouse_event.x, mouse_event.y);
                            Internal::DeviceToLogical(mouse_event, scale_factor);
                            mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                            browser_host->SendMouseMoveEvent(mouse_event, false);
                        }
                    }
                    break;
                }

                case UIEVENT_MOUSELEAVE: {
                    if (mouse_tracking_) {
                        // Stop tracking mouse leave.
                        TRACKMOUSEEVENT tme;
                        tme.cbSize = sizeof(TRACKMOUSEEVENT);
                        tme.dwFlags = TME_LEAVE & TME_CANCEL;
                        tme.hwndTrack = hwnd;
                        TrackMouseEvent(&tme);
                        mouse_tracking_ = false;
                    }

                    if (browser_host) {
                        // Determine the cursor position in screen coordinates.
                        POINT p;
                        ::GetCursorPos(&p);
                        ::ScreenToClient(hwnd, &p);

                        CefMouseEvent mouse_event;
                        mouse_event.x = p.x;
                        mouse_event.y = p.y;
                        Internal::DeviceToLogical(mouse_event, scale_factor);
                        mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                        browser_host->SendMouseMoveEvent(mouse_event, true);
                    }
                }
                break;

                case UIEVENT_SCROLLWHEEL:
                    if (browser_host) {
                        POINT screen_point = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                        HWND scrolled_wnd = ::WindowFromPoint(screen_point);
                        if (scrolled_wnd != hwnd)
                            break;

                        ScreenToClient(hwnd, &screen_point);
                        screen_point.x -= pos.left;
                        screen_point.y -= pos.top;
                        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

                        CefMouseEvent mouse_event;
                        mouse_event.x = screen_point.x;
                        mouse_event.y = screen_point.y;
                        //ApplyPopupOffset(mouse_event.x, mouse_event.y);
                        Internal::DeviceToLogical(mouse_event, scale_factor);
                        mouse_event.modifiers = Internal::GetCefMouseModifiers(wParam);
                        browser_host->SendMouseWheelEvent(mouse_event,
                                                          Internal::IsKeyDown(VK_SHIFT) ? delta : 0,
                                                          !Internal::IsKeyDown(VK_SHIFT) ? delta : 0);
                    }
                    break;
            }
        }

        void OnKeyEvent(UINT message, WPARAM wParam, LPARAM lParam) {
            if (!m_browser)
                return;

            CefKeyEvent event;
            event.windows_key_code = wParam;
            event.native_key_code = lParam;
            event.is_system_key = (message == UIEVENT_SYSCHAR || message == UIEVENT_SYSKEYDOWN ||
                                   message == UIEVENT_SYSKEYUP);

            if (message == UIEVENT_KEYDOWN || message == UIEVENT_SYSKEYDOWN)
                event.type = KEYEVENT_RAWKEYDOWN;
            else if (message == UIEVENT_KEYUP || message == UIEVENT_SYSKEYUP)
                event.type = KEYEVENT_KEYUP;
            else
                event.type = KEYEVENT_CHAR;
            event.modifiers = Internal::GetCefKeyboardModifiers(wParam, lParam);

            m_browser->GetHost()->SendKeyEvent(event);
        }

        void OnFocus(bool setFocus) {
            if (m_browser)
                m_browser->GetHost()->SendFocusEvent(setFocus);
        }

        void OnSize() {
            if (m_browser)
                m_browser->GetHost()->WasResized();
        }

        void SetAllowProtocols(const std::vector<std::string> vAllowProtocols) {
            m_vAllowProtocols = vAllowProtocols;
            if (m_ClientHandler) {
                m_ClientHandler->SetAllowProtocols(vAllowProtocols);
            }
        }

        std::vector<std::string> GetAllowProtocols() const {
            return m_vAllowProtocols;
        }

        void SetFPS(int fps) {
            m_iFPS = fps;
        }

        int GetFPS() const {
            return m_iFPS;
        }

        void SetCefBkColor(DWORD dwBackColor) {
            m_dwCefBkColor = dwBackColor;
        }

        DWORD GetCefBkColor() const {
            return m_dwCefBkColor;
        }

        void SetBkTransparent(bool b) {
            m_bBkTransparent = b;
        }

        bool GetBkTransparent() const {
            return m_bBkTransparent;
        }
      public:
        CCefUI *m_pParent;
        HDC m_hMemoryDC;
        HBITMAP m_hBitmap;
        void *m_pBuffer;
        int m_iMemoryBitmapWidth;
        int m_iMemoryBitmapHeight;
        ppx::base::CriticalSection m_csBuf;

        CDuiString m_strInitUrl;
        uint32_t m_iRandomID;

        std::vector<std::string> m_vAllowProtocols;
        int m_iFPS;
        bool m_bBkTransparent;
        DWORD m_dwCefBkColor;

        // Mouse state tracking.
        POINT last_mouse_pos_;
        POINT current_mouse_pos_;
        POINT mouse_pos_;
        bool mouse_rotation_;
        bool mouse_tracking_;
        int last_click_x_;
        int last_click_y_;
        CefBrowserHost::MouseButtonType last_click_button_;
        int last_click_count_;
        double last_click_time_;
        bool last_mouse_down_on_view_;

        CefRefPtr<Internal::ClientHandlerOsr> m_ClientHandler;
        CefRefPtr<CefBrowser> m_browser;
        Internal::CefDevToolsWnd *m_pDevToolsWnd;
    };

    IMPLEMENT_DUICONTROL(CCefUI)

    CCefUI::CCefUI() :
        m_pImpl(new CCefUIImpl(this))
        , m_hCreated(false) {

    }

    CCefUI::~CCefUI() {
        if (m_pImpl) {
            delete m_pImpl;
            m_pImpl = NULL;
        }
    }

    LPCTSTR CCefUI::GetClass() const {
        return DUI_CTR_CEF;
    }

    LPVOID CCefUI::GetInterface(LPCTSTR pstrName) {
        if (_tcsicmp(pstrName, DUI_CTR_CEF) == 0)
            return static_cast<CCefUI *>(this);

        return CContainerUI::GetInterface(pstrName);
    }

    void CCefUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
        if (_tcsicmp(pstrName, _T("url")) == 0)
            SetUrl(pstrValue);
        else if (_tcsicmp(pstrName, _T("transparent")) == 0)
            m_pImpl->SetBkTransparent(_tcsicmp(pstrValue, _T("true")) == 0);
        else if (_tcsicmp(pstrName, _T("fps")) == 0)
            m_pImpl->SetFPS(_ttoi(pstrValue));
        else if (_tcsicmp(pstrName, _T("cefbkcolor")) == 0) {
            while (*pstrValue > _T('\0') && *pstrValue <= _T(' ')) pstrValue = ::CharNext(pstrValue);

            if (*pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);

            LPTSTR pstr = NULL;
            DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
            m_pImpl->SetCefBkColor(clrColor);
        } else
            CControlUI::SetAttribute(pstrName, pstrValue);
    }

    void CCefUI::SetPos(RECT rc, bool bNeedInvalidate /* = true */) {
        CContainerUI::SetPos(rc, bNeedInvalidate);
        if (!m_hCreated) {
            m_hCreated = true;
            m_pImpl->CreateBrowser();
        } else {
            m_pImpl->OnSize();
        }
    }

    void CCefUI::DoInit() {
        if (!m_hCreated) {
            m_hCreated = true;
            m_pImpl->CreateBrowser();
        }
    }

    bool CCefUI::DoPaint(HDC hDC, const RECT &rcPaint, CControlUI *pStopControl) {
        bool bRet = CContainerUI::DoPaint(hDC, rcPaint, pStopControl);
        m_pImpl->DoPaint(hDC);
        return bRet;
    }

    void CCefUI::DoEvent(TEventUI &event) {
        if (event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_MOUSELEAVE
                || event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_BUTTONUP
                || event.Type == UIEVENT_RBUTTONDOWN || event.Type == UIEVENT_RBUTTONUP
                || event.Type == UIEVENT_MBUTTONDOWN || event.Type == UIEVENT_MBUTTONUP || event.Type == UIEVENT_SCROLLWHEEL) {
            m_pImpl->OnMouseEvent(event.Type, event.wParam, event.lParam);
        } else if (event.Type == UIEVENT_KEYDOWN || event.Type == UIEVENT_KEYUP
                   || event.Type == UIEVENT_CHAR || event.Type == UIEVENT_SYSCHAR
                   || event.Type == UIEVENT_SYSKEYDOWN || event.Type == UIEVENT_SYSKEYUP) {
            m_pImpl->OnKeyEvent(event.Type, event.wParam, event.lParam);
        } else if (event.Type == UIEVENT_SETFOCUS || event.Type == UIEVENT_KILLFOCUS) {
            m_pImpl->OnFocus(event.Type == UIEVENT_SETFOCUS);
        }
    }

    bool CCefUI::GetBkTransparent() const {
        return m_pImpl->GetBkTransparent();
    }

    void CCefUI::SetResourceResponseCallback(ResourceResponseCallback cb) {
        m_ResourceRspCB = cb;
    }

    CCefUI::ResourceResponseCallback CCefUI::GetResourceResponseCallback() const {
        return m_ResourceRspCB;
    }

    void CCefUI::SetJSCallback(JSCallback cb) {
        m_JSCB = cb;
    }

    CCefUI::JSCallback CCefUI::GetJSCallback() const {
        return m_JSCB;
    }

    void CCefUI::SetUrl(const CDuiString &url) {
        m_strUrl = url;
        m_pImpl->SetUrl(url);
    }

    DuiLib::CDuiString CCefUI::GetUrl() const {
        return m_strUrl;
    }

    int CCefUI::GetFPS() const {
        return m_pImpl->GetFPS();
    }

    DWORD CCefUI::GetCefBkColor() const {
        return m_pImpl->GetCefBkColor();
    }

    void CCefUI::GoBack() {
        m_pImpl->GoBack();
    }

    void CCefUI::GoForward() {
        m_pImpl->GoForward();
    }

    void CCefUI::Reload() {
        m_pImpl->Reload();
    }

    void CCefUI::ShowDevTools() {
        m_pImpl->ShowDevTools();
    }

    void CCefUI::CloseDevTools() {
        m_pImpl->CloseDevTools();
    }

    bool CCefUI::CallJavascriptFunction(const std::string &strFuncName, const std::vector<CLiteVariant> &args) {
        return m_pImpl->CallJavascriptFunction(strFuncName, args);
    }

    void CCefUI::SetAllowProtocols(const std::vector<std::string> vAllowProtocols) {
        m_pImpl->SetAllowProtocols(vAllowProtocols);
    }

    std::vector<std::string> CCefUI::GetAllowProtocols() const {
        return m_pImpl->GetAllowProtocols();
    }
}
#endif