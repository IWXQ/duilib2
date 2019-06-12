#pragma once

class CefTestWnd : 
    public WindowImplBase {
public:
    CefTestWnd();
    ~CefTestWnd();

    CDuiString GetSkinFile() override {
        return TEXT("cef.xml");
    }

    LPCTSTR GetWindowClassName() const override {
        return TEXT("DlgCEFBA9B2BC0");
    }

    void Notify(TNotifyUI& msg) override;
    void OnFinalMessage(HWND hWnd) override;
private:
    void OnWindowInit();
    void TestCallJS(int browser_id);
private:

	CCefUI* web1_;
	CCefUI* web2_;

	CButtonUI* btn_min_;
	CButtonUI* btn_close_;

	CButtonUI* btn_go_back_1_;
	CButtonUI* btn_go_foward_1_;
	CButtonUI* btn_refresh_1_;
	CButtonUI* btn_go_1_;
	CButtonUI* btn_open_devtools_1_;
	CButtonUI* btn_close_devtools_1_;
	CEditUI*   edt_url_1_;

	CButtonUI* btn_go_back_2_;
	CButtonUI* btn_go_foward_2_;
	CButtonUI* btn_refresh_2_;
	CButtonUI* btn_go_2_;
	CButtonUI* btn_open_devtools_2_;
	CButtonUI* btn_close_devtools_2_;
	CEditUI*   edt_url_2_;
};

