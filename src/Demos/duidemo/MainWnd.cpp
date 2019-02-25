#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "SkinFrame.h"
#include <thread>

DUI_BEGIN_MESSAGE_MAP(CMainPage, CNotifyPump)
DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, OnSelectChanged)
DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK, OnItemClick)
DUI_END_MESSAGE_MAP()

CMainPage::CMainPage() {
    m_pPaintManager = NULL;
}

void CMainPage::SetPaintMagager(CPaintManagerUI *pPaintMgr) {
    m_pPaintManager = pPaintMgr;
}


void CMainPage::OnSelectChanged( TNotifyUI &msg ) {

}

void CMainPage::OnItemClick( TNotifyUI &msg ) {

}

DUI_BEGIN_MESSAGE_MAP(CMainWnd, WindowImplBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

CMainWnd::CMainWnd() : 
    m_pList(NULL),
    m_pEditH(NULL),
    m_pEditS(NULL),
    m_pEditL(NULL),
    m_pHrlTitle(NULL) {
    m_pMenu = NULL;

    m_MainPage.SetPaintMagager(&m_PaintManager);
    AddVirtualWnd(_T("mainpage"), &m_MainPage);
}

CMainWnd::~CMainWnd() {
    CMenuWnd::DestroyMenu();

    if(m_pMenu != NULL) {
        delete m_pMenu;
        m_pMenu = NULL;
    }

    RemoveVirtualWnd(_T("mainpage"));
}


void CMainWnd::InitWindow() {
    SetIcon(IDI_ICON1);
    CResourceManager::GetInstance()->SetTextQueryInterface(this);
    CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.xml"));
    CSkinManager::GetSkinManager()->AddReceiver(this);

    m_pBtnMax = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btnMax")));
    m_pBtnRestore = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btnRestore")));
    m_pBtnMin = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btnMin")));
    m_pBtnSkin = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btnSkin")));
    m_pList = static_cast<CListUI *>(m_PaintManager.FindControl(TEXT("listView")));
    m_pHrlTitle = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(TEXT("hrlTitle")));
    m_pEditH = static_cast<CEditUI*>(m_PaintManager.FindControl(TEXT("editH")));
    m_pEditS = static_cast<CEditUI*>(m_PaintManager.FindControl(TEXT("editS")));
    m_pEditL = static_cast<CEditUI*>(m_PaintManager.FindControl(TEXT("editL")));


    if (m_pHrlTitle) {
        DWORD dwBkColor = m_pHrlTitle->GetBkColor();
        float h = 0.f;
        float s = 0.f;
        float l = 0.f;
        CRenderEngine::RGBtoHSL(dwBkColor, &h, &s, &l);

        if (m_pEditH)
            m_pEditH->SetText(base::MakeString(TEXT("%.0f"), h).GetDataPointer());
        if (m_pEditS)
            m_pEditS->SetText(base::MakeString(TEXT("%.3f"), s).GetDataPointer());
        if (m_pEditL)
            m_pEditL->SetText(base::MakeString(TEXT("%.3f"), l).GetDataPointer());
    }

    // 初始化WebBrowser控件
    CWebBrowserUI *pBrowser1 = static_cast<CWebBrowserUI *>(m_PaintManager.FindControl(_T("oneclick_browser1")));
    if (pBrowser1) {
        pBrowser1->SetWebBrowserEventHandler(this);
        pBrowser1->NavigateUrl(_T("http://blog.csdn.net/china_jeffery"));
    }

    CWebBrowserUI *pBrowser2 = static_cast<CWebBrowserUI *>(m_PaintManager.FindControl(_T("oneclick_browser2")));
    if (pBrowser2) {
        pBrowser2->SetWebBrowserEventHandler(this);
        pBrowser2->NavigateUrl(_T("http://blog.csdn.net/china_jeffery"));
    }


    CColorPaletteUI *pColorPalette = (CColorPaletteUI *)m_PaintManager.FindControl(_T("Pallet"));
    if (pColorPalette) {
        pColorPalette->SetSelectColor(0xff0199cb);
    }


    m_trayIcon.CreateTrayIcon(m_hWnd, IDI_ICON1, _T("DuiLib Demo"));
}


void CMainWnd::Notify(TNotifyUI &msg) {
    base::String strName = msg.pSender->GetName();

    if (msg.sType == _T("windowinit")) {
    } 
    else if (msg.sType == DUI_MSGTYPE_COLORCHANGED) {
        if (strName == TEXT("Pallet")) {
            CControlUI *pRoot = m_PaintManager.FindControl(_T("root"));

            if (pRoot != NULL) {
                CColorPaletteUI *pColorPalette = static_cast<CColorPaletteUI *>(msg.pSender);

                if (pColorPalette) {
                    pRoot->SetBkColor(pColorPalette->GetSelectColor());
                    pRoot->SetBkImage(_T(""));
                }
            }
        }
    } 
    else if (msg.sType == DUI_MSGTYPE_ITEMACTIVATE) {
    } 
    else if (msg.sType == DUI_MSGTYPE_SHOWACTIVEX) {
        if (strName.CompareNoCase(_T("ani_flash")) == 0) {
            IShockwaveFlash *pFlash = NULL;
            CActiveXUI *pActiveX = static_cast<CActiveXUI *>(msg.pSender);
            pActiveX->GetControl(__uuidof(IShockwaveFlash), (void **)&pFlash);

            if (pFlash != NULL) {
                pFlash->put_WMode(_bstr_t(_T("Transparent")));
                pFlash->put_Movie(_bstr_t((CPaintManagerUI::GetInstancePath() + _T("\\skin\\duidemo\\other\\waterdrop.swf")).GetDataPointer()));
                pFlash->DisableLocalSecurity();
                pFlash->put_AllowScriptAccess(L"always");
                BSTR response;
                pFlash->CallFunction(L"<invoke name=\"setButtonText\" returntype=\"xml\"><arguments><string>Click me!</string></arguments></invoke>", &response);
                pFlash->Release();
            }
        }
    } 
    else if (msg.sType == DUI_MSGTYPE_SELECTCHANGED) {
        CTabLayoutUI *pTabSwitch = static_cast<CTabLayoutUI *>(m_PaintManager.FindControl(_T("tabMain")));

        if (pTabSwitch) {
            if (strName.CompareNoCase(_T("tabBasicControl")) == 0)
                pTabSwitch->SelectItem(0);
            else if (strName.CompareNoCase(_T("tabAdvanceControl")) == 0)
                pTabSwitch->SelectItem(1);
            else if (strName.CompareNoCase(_T("tabExtensionControl")) == 0)
                pTabSwitch->SelectItem(2);
            else if (strName.CompareNoCase(_T("tabAnimationControl")) == 0)
                pTabSwitch->SelectItem(3);
            else if (strName.CompareNoCase(_T("tabSplitWindow")) == 0)
                pTabSwitch->SelectItem(4);
        }
    } 
    else if (msg.sType == DUI_MSGTYPE_VALUECHANGED) {
        if (strName == TEXT("slider")) {
            int value = 0;
            CProgressUI *pSlider = static_cast<CProgressUI *>(msg.pSender);
            if (pSlider)
                value = pSlider->GetValue();

            CProgressUI *pProColor = static_cast<CProgressUI *>(m_PaintManager.FindControl(_T("progressColor")));
            if (pProColor) {
                pProColor->SetValue(value);
                pProColor->SetText(base::MakeString(TEXT("%d%%"), value).GetDataPointer());
            }
        }
    } 
    else if (msg.sType == DUI_MSGTYPE_TEXTCHANGED) {
        float h, s, l = 0.f;
        if (msg.pSender == m_pEditH || msg.pSender == m_pEditS || msg.pSender == m_pEditL) {
            TCHAR *end;
            h = _tcstof(m_pEditH->GetText().GetDataPointer(), &end);
            s = _tcstof(m_pEditS->GetText().GetDataPointer(), &end);
            l = _tcstof(m_pEditL->GetText().GetDataPointer(), &end);

            CPaintManagerUI::SetHSL(true, h, s*100, l*100);
        }
    }
    else if (msg.sType == DUI_MSGTYPE_PREDROPDOWN) {
    }

    return WindowImplBase::Notify(msg);
}

void CMainWnd::OnClick(TNotifyUI &msg) {
    base::String strName = msg.pSender->GetName();

    if (strName.CompareNoCase(_T("btnClose")) == 0) {
        if (MSGID_YES == CMsgWnd::MessageBox(m_hWnd, _T("DuiLib Demo"), _T("Are you sure to exit DuiLib Demo?"))) {
            Close();
        }
    } 
    else if (msg.pSender == m_pBtnMin) {
        SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
    }
    else if (msg.pSender == m_pBtnMax) {
        SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);

        if (m_pBtnMax)
            m_pBtnMax->SetVisible(false);

        if (m_pBtnRestore)
            m_pBtnRestore->SetVisible(true);
    } 
    else if (msg.pSender == m_pBtnRestore) {
        SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);

        if (m_pBtnMax)
            m_pBtnMax->SetVisible(true);

        if (m_pBtnRestore)
            m_pBtnRestore->SetVisible(false);
    } 
    else if (msg.pSender == m_pBtnSkin) {
        new CSkinFrame(m_hWnd, m_pBtnSkin);
    } 
    else if (strName.CompareNoCase(_T("btnMenu")) == 0) {
        CMenuWnd::GetGlobalContextMenuObserver().SetMenuItemInfo(&m_MenuInfos);

        if (m_pMenu != NULL) {
            delete m_pMenu;
            m_pMenu = NULL;
        }

        m_pMenu = new CMenuWnd();
        CDuiPoint point;
        ::GetCursorPos(&point);
        m_pMenu->Init(NULL, _T("menu.xml"), point, &m_PaintManager);
        // 设置状态
     /*   CMenuWnd::SetMenuItemInfo(_T("qianting"), true);

        CMenuUI *rootMenu = m_pMenu->GetMenuUI();

        if (rootMenu != NULL) {
            CMenuElementUI *pNew = new CMenuElementUI;
            pNew->SetName(_T("Menu_Dynamic"));
            pNew->SetText(_T("动态一级菜单"));
            pNew->SetShowExplandIcon(true);
            pNew->SetIcon(_T("WebSit.png"));
            pNew->SetIconSize(16, 16);
            rootMenu->Add(pNew);

            CMenuElementUI *pNew2 = new CMenuElementUI;
            pNew2->SetName(_T("Menu_Dynamic"));
            pNew2->SetText(_T("动态一级菜单2"));
            rootMenu->AddAt(pNew2, 2);
        }
*/
        // 动态添加后重新设置菜单的大小
        m_pMenu->ResizeMenu();
    } 
    else if (strName.CompareNoCase(_T("btnDpi")) == 0) {
        m_PaintManager.SetDPI(_ttoi(msg.pSender->GetUserData().GetDataPointer()));
    }
    else if (strName.CompareNoCase(TEXT("btnDynamicAdd1")) == 0) {
        AddListItem(1);
    } 
    else if (strName.CompareNoCase(TEXT("btnDynamicAdd100")) == 0) {
        AddListItem(100);
    }
    else if (strName.CompareNoCase(TEXT("btnSelectAll")) == 0) {
        if (m_pList)
            m_pList->SelectAllItems();
    }
    else if (strName.CompareNoCase(TEXT("btnDelSelected")) == 0) {
        if (m_pList) {
            int iCount = m_pList->GetCount();
            int i = 0;
            CListContainerElementUI* pItem = NULL;

            while (i < iCount) {
                pItem = static_cast<CListContainerElementUI*>(m_pList->GetItemAt(i));
                if (pItem && pItem->IsSelected()) {
                    if (m_pList->RemoveAt(i, false)) {
                        iCount = m_pList->GetCount();
                        i = 0;
                        continue;
                    }
                }
                i++;
            }
        }
    }
    else if (strName.CompareNoCase(TEXT("btnDel")) == 0) {
        
    }
	else if (strName.CompareNoCase(TEXT("btnPostTask")) == 0) {
		std::thread t = std::thread([this]() {
			PostTaskToUIThread(base::BindLambda([this]() {
				assert(IsInUIThread());
				CButtonUI *btnPostTask = static_cast<CButtonUI *>(m_PaintManager.FindControl(TEXT("btnPostTask")));
				if (btnPostTask) {
					btnPostTask->SetEnabled(false);
				}
			}));

			for (int i = 0; i < 1000; i++) {
				Sleep(10);
				UpdateText(base::MakeString(TEXT("%d"), i));
			}

			PostTaskToUIThread(base::BindLambda([this]() {
				assert(IsInUIThread());
				CButtonUI *btnPostTask = static_cast<CButtonUI *>(m_PaintManager.FindControl(TEXT("btnPostTask")));
				if (btnPostTask) {
					btnPostTask->SetEnabled(true);
				}
			}));
		});
		t.detach();
	}
}

CControlUI *CMainWnd::CreateControl(LPCTSTR pstrClass) {
    if(lstrcmpi(pstrClass, _T("CircleProgress" )) == 0) {
        return new CCircleProgressUI();
    }

    return NULL;
}

BOOL CMainWnd::Receive(SkinChangedParam param) {
    CControlUI *pRoot = m_PaintManager.FindControl(_T("root"));

    if( pRoot != NULL ) {
        if( param.bColor ) {
            pRoot->SetBkColor(param.bkcolor);
            pRoot->SetBkImage(_T(""));
        } else {
            pRoot->SetBkColor(0);
            pRoot->SetBkImage(param.bgimage.GetDataPointer());
            //m_PaintManager.SetLayeredImage(param.bgimage);
        }
    }

    return TRUE;
}

HRESULT STDMETHODCALLTYPE CMainWnd::UpdateUI( void) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMainWnd::GetHostInfo(CWebBrowserUI *pWeb,
        /* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo) {
    if (pInfo != NULL) {
        pInfo->dwFlags |= DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_SCROLL_NO;
    }

    return S_OK;
}
HRESULT STDMETHODCALLTYPE CMainWnd::ShowContextMenu(CWebBrowserUI *pWeb,
        /* [in] */ DWORD dwID,
        /* [in] */ POINT __RPC_FAR *ppt,
        /* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
        /* [in] */ IDispatch __RPC_FAR *pdispReserved) {
    return E_NOTIMPL;
    //返回 E_NOTIMPL 正常弹出系统右键菜单
    //返回S_OK 则可屏蔽系统右键菜单
}

void CMainWnd::UpdateText(const base::String &str) {
    if (PostTaskWhenNotInUIThread(base::Bind(&CMainWnd::UpdateText, base::Unretained(this), str))) {
        return;
    }
	assert(IsInUIThread());
	CButtonUI *btnPostTask = static_cast<CButtonUI *>(m_PaintManager.FindControl(TEXT("btnPostTask")));
    if (btnPostTask) {
		btnPostTask->SetText(str.GetDataPointer());
    }
}

void CMainWnd::AddListItem(int iCount) {
    static int iHasAdd = 0;

    for (int i = 0; i < iCount; i++) {
        iHasAdd++;
        CDialogBuilder builder;
        CListContainerElementUI *pListItem = (CListContainerElementUI *)builder.Create(_T("listitem.xml"), NULL, this, &m_PaintManager, NULL);
        CLabelUI *pLabel = NULL;

        pLabel = static_cast<CLabelUI *>(pListItem->FindSubControl(_T("lblName")));
        if (pLabel)
            pLabel->SetText(base::MakeString(TEXT("Name[%d]"), iHasAdd).GetDataPointer());

        pLabel = static_cast<CLabelUI *>(pListItem->FindSubControl(_T("lblID")));
        if (pLabel)
            pLabel->SetText(base::MakeString(TEXT("No.%06d"), 1000 + iHasAdd).GetDataPointer());

        pLabel = static_cast<CLabelUI *>(pListItem->FindSubControl(_T("lblScore")));
        if (pLabel) {
            srand((unsigned int)time(NULL));
            pLabel->SetText(base::MakeString(TEXT("%d"), rand() % 100).GetDataPointer());
        }

        if (m_pList)
            m_pList->Add(pListItem);
    }
}

base::String CMainWnd::GetSkinFile() {
    return _T("XML_MAIN");
}

LPCTSTR CMainWnd::GetWindowClassName() const {
    return _T("MainWnd");
}

UINT CMainWnd::GetClassStyle() const {
    return CS_DBLCLKS;
}

void CMainWnd::OnFinalMessage(HWND hWnd) {
    __super::OnFinalMessage(hWnd);
}

LPCTSTR CMainWnd::QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType) {
    base::String sLanguage = CResourceManager::GetInstance()->GetLanguage();

    if(sLanguage == _T("en")) {
        if(lstrcmpi(lpstrId, _T("titletext")) == 0) {
            return _T("Duilib Demo v1.1");
        } 
        else if(lstrcmpi(lpstrId, _T("hometext")) == 0) {
            return _T("{a}Home Page{/a}");
        }
    } 
    else {
        if(lstrcmpi(lpstrId, _T("titletext")) == 0) {
            return _T("Duilib 使用演示 v1.1");
        } 
        else if(lstrcmpi(lpstrId, _T("hometext")) == 0) {
            return _T("{a}演示官网{/a}");
        }
    }

    return NULL;
}


LRESULT CMainWnd::ResponseDefaultKeyEvent(WPARAM wParam) {
    if (wParam == VK_ESCAPE) {
        return TRUE;
    }
    else if (wParam == VK_RETURN) {
        return TRUE;
    }
    return FALSE;
}

LRESULT CMainWnd::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled) {
    m_trayIcon.DeleteTrayIcon();
    bHandled = FALSE;
    PostQuitMessage(0);
    return 0;
}

LRESULT CMainWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
    if(uMsg == WM_DESTROY) {
        ::PostQuitMessage(0L);
        bHandled = TRUE;
        return 0;
    } 
    else if(uMsg == WM_TIMER) {
        bHandled = FALSE;
    } 
    else if(uMsg == WM_SHOWWINDOW) {
        bHandled = FALSE;

        if(m_pBtnMin)
            m_pBtnMin->NeedParentUpdate();

        InvalidateRect(m_hWnd, NULL, TRUE);
    } 
    else if(uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN) {
    } 
    else if (uMsg == WM_MENUCLICK) {
        MenuCmd *pMenuCmd = (MenuCmd *)wParam;

        if(pMenuCmd != NULL) {
            BOOL bChecked = pMenuCmd->bChecked;
            base::String sMenuName = pMenuCmd->szName;
            base::String sUserData = pMenuCmd->szUserData;
            base::String sText = pMenuCmd->szText;
            m_PaintManager.DeletePtr(pMenuCmd);

            if(sMenuName.CompareNoCase(_T("lan")) == 0) {
                static bool bEn = false;

                if(!bEn) {
                    CResourceManager::GetInstance()->SetLanguage(_T("en"));
                    CResourceManager::GetInstance()->LoadLanguage(_T("lan_en.xml"));
                } else {
                    CResourceManager::GetInstance()->SetLanguage(_T("cn_zh"));
                    CResourceManager::GetInstance()->LoadLanguage(_T("lan_cn.xml"));
                }

                bEn = !bEn;
                CResourceManager::GetInstance()->ReloadText();
                InvalidateRect(m_hWnd, NULL, TRUE);
                m_PaintManager.NeedUpdate();
            } else if(sMenuName == _T("exit")) {
                Close(0);
            } else {
                CMsgWnd::MessageBox(m_hWnd, NULL, sText.GetDataPointer());
            }
        }

        bHandled = TRUE;
        return 0;
    } 
    else if(uMsg == UIMSG_TRAYICON) {
        UINT uIconMsg = (UINT)lParam;

        if(uIconMsg == WM_LBUTTONUP) {
            BOOL bVisible = IsWindowVisible(m_hWnd);
            ::ShowWindow(m_hWnd, !bVisible ?  SW_SHOW : SW_HIDE);
        } 
        else if(uIconMsg == WM_RBUTTONUP) {
            if(m_pMenu != NULL) {
                delete m_pMenu;
                m_pMenu = NULL;
            }

            m_pMenu = new CMenuWnd();
            CDuiPoint point;
            ::GetCursorPos(&point);
            point.y -= 100;
            m_pMenu->Init(NULL, _T("menu.xml"), point, &m_PaintManager);
            // 动态添加后重新设置菜单的大小
            m_pMenu->ResizeMenu();
        }
    }
    else if (uMsg == WM_DPICHANGED) {
        m_PaintManager.SetDPI(LOWORD(wParam));  // Set the new DPI, retrieved from the wParam
        m_PaintManager.ResetDPIAssets();
        RECT *const prcNewWindow = (RECT *)lParam;
        SetWindowPos(m_hWnd, NULL, prcNewWindow->left, prcNewWindow->top, prcNewWindow->right - prcNewWindow->left, prcNewWindow->bottom - prcNewWindow->top, SWP_NOZORDER | SWP_NOACTIVATE);

        if (m_PaintManager.GetRoot() != NULL)
            m_PaintManager.GetRoot()->NeedUpdate();
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CMainWnd::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled) {
    bHandled = FALSE;
    return 0;
}
