#include "StdAfx.h"
#include "CefTestWnd.h"

CefTestWnd::CefTestWnd() :
	btn_go_back_1_(nullptr)
	, btn_go_foward_1_(nullptr)
	, btn_refresh_1_(nullptr)
	, btn_go_1_(nullptr)
	, edt_url_1_(nullptr)
	, btn_open_devtools_1_(nullptr)
	, btn_close_devtools_1_(nullptr)
	, btn_go_back_2_(nullptr)
	, btn_go_foward_2_(nullptr)
	, btn_refresh_2_(nullptr)
	, btn_go_2_(nullptr)
	, edt_url_2_(nullptr)
	, btn_open_devtools_2_(nullptr)
	, btn_close_devtools_2_(nullptr)
	, btn_min_(nullptr)
	, btn_close_(nullptr)
	, btn_call_js_1_(nullptr)
	, btn_call_js_2_(nullptr)
{

}

CefTestWnd::~CefTestWnd() {

}

void CefTestWnd::Notify(TNotifyUI& msg) {
    WindowImplBase::Notify(msg);
    CDuiString strSenderName;

    if (msg.sType == DUI_MSGTYPE_WINDOWINIT) {
        OnWindowInit();
    }
    else if (msg.sType == DUI_MSGTYPE_CLICK) {
		if (msg.pSender == btn_go_1_) {
			if (edt_url_1_->GetText().GetLength() > 0) {
				web1_->SetUrl(edt_url_1_->GetText());
			}
		}
		else if (msg.pSender == btn_go_2_) {
			if (edt_url_2_->GetText().GetLength() > 0) {
				web2_->SetUrl(edt_url_2_->GetText());
			}
		}
		else if (msg.pSender == btn_go_back_1_) {
			web1_->GoBack();
		}
		else if (msg.pSender == btn_go_back_2_) {
			web2_->GoBack();
		}
		else if (msg.pSender == btn_go_foward_1_) {
			web1_->GoForward();
		}
		else if (msg.pSender == btn_go_foward_2_) {
			web2_->GoForward();
		}
		else if (msg.pSender == btn_refresh_1_) {
			web1_->Reload();
		}
		else if (msg.pSender == btn_refresh_2_) {
			web2_->Reload();
		}
		else if (msg.pSender == btn_open_devtools_1_) {
			web1_->ShowDevTools();
		}
		else if (msg.pSender == btn_open_devtools_2_) {
			web2_->ShowDevTools();
		}
		else if (msg.pSender == btn_close_devtools_1_) {
			web1_->CloseDevTools();
		}
		else if (msg.pSender == btn_close_devtools_2_) {
			web2_->CloseDevTools();
		}
		else if (msg.pSender == btn_call_js_1_) {

		}
		else if (msg.pSender == btn_call_js_2_) {
			Web2CallJS();
		}
		if (msg.pSender == btn_close_) {
			Close();
		}
		else if (msg.pSender == btn_min_) {
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
    }
	else if (msg.sType == DUI_MSGTYPE_JAVASCRIPT_NOTIFY) {
		CDuiString strWebName = msg.pSender->GetName();
		CDuiString strBusiness = *((CDuiString*)msg.wParam);
		std::vector<VARIANT> vars = *((std::vector<VARIANT>*)msg.lParam);

		std::stringstream ss;
		ss << "web: " << TCHARToAnsi(strWebName.GetData()) << std::endl;
		ss << "business name: " << ppx::base::UnicodeToAnsi(strBusiness.GetData()) << std::endl;
		for (auto it : vars) {
			if (it.vt == VT_BSTR && it.bstrVal)
				ss << ppx::base::Utf8ToAnsi(BSTRToString(&it)) << std::endl;
			else
				ss << it.intVal << std::endl;
		}
		MessageBoxA(NULL, ss.str().c_str(), "JS Notify", MB_OK | MB_ICONINFORMATION);
	}
}

void CefTestWnd::OnWindowInit() {
	btn_go_back_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnBrowserBack1")));
	btn_go_foward_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnBrowserForward1")));
	btn_refresh_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnBrowserRefresh1")));
	btn_go_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnGo1")));
	btn_open_devtools_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnOpenDevTools1")));
	btn_close_devtools_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnCloseDevTools1")));
	btn_call_js_1_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnCallJS1")));
	edt_url_1_ = static_cast<CEditUI*>(m_PaintManager.FindControl(TEXT("edtUrl1")));
	web1_ = static_cast<CCefUI*>(m_PaintManager.FindControl(TEXT("web1")));

	btn_go_back_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnBrowserBack2")));
	btn_go_foward_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnBrowserForward2")));
	btn_refresh_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnBrowserRefresh2")));
	btn_go_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnGo2")));
	btn_open_devtools_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnOpenDevTools2")));
	btn_close_devtools_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnCloseDevTools2")));
	btn_call_js_2_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnCallJS2")));
	edt_url_2_ = static_cast<CEditUI*>(m_PaintManager.FindControl(TEXT("edtUrl2")));
	web2_ = static_cast<CCefUI*>(m_PaintManager.FindControl(TEXT("web2")));

	btn_min_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnMin")));
	btn_close_ = static_cast<CButtonUI*>(m_PaintManager.FindControl(TEXT("btnClose")));


	web1_->SetUrl(TEXT("https://pinyin.sogou.com/"));
	web2_->SetUrl((ppx::base::GetCurrentProcessDirectoryW() + L"..\\..\\..\\test-resource\\test.html").c_str());

	web1_->SetResourceResponseCallback([](const std::string &url, int status) {
		ppx::base::TraceMsgA("web1: %s [%d]\n", url.c_str(), status);
	});

	web2_->SetResourceResponseCallback([](const std::string &url, int status) {
		ppx::base::TraceMsgA("web2: %s [%d]\n", url.c_str(), status);
	});
}


void CefTestWnd::OnFinalMessage(HWND hWnd) {
    WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

void CefTestWnd::Web2CallJS() {
    std::vector<VARIANT> args;
    VARIANT arg0;
    VariantInit(&arg0);
    arg0.vt = VT_I4;
    arg0.intVal = 10;
    args.push_back(arg0);

    VARIANT arg1;
    VariantInit(&arg1);
    arg1.vt = VT_BSTR;
    arg1.bstrVal = SysAllocString(L"测试TEST");

    args.push_back(arg1);

    bool ret = web2_->CallJavascriptFunction(TEXT("cpp2js_test"), args);

    if (arg1.bstrVal) {
        SysFreeString(arg1.bstrVal);
    }
}
