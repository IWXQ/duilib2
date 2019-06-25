/*******************************************************************************
* Copyright (C) 2018 - 2020, winsoft666, <winsoft666@outlook.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
* Expect bugs
*
* Please use and enjoy. Please let me know of any bugs/improvements
* that you have found/implemented and I will fix/incorporate them into this
* file.
*******************************************************************************/
#ifndef __UICEF_H__
#define __UICEF_H__
#pragma once
#ifdef UILIB_WITH_CEF
#include <functional>

namespace DuiLib {
	class UILIB_API CCefUI : public CContainerUI {
		DECLARE_DUICONTROL(CCefUI)
	public:
		typedef std::function<void(const std::string &url, int response_status)> ResourceResponseCallback;

		CCefUI();
		~CCefUI();

		LPCTSTR GetClass() const override;
		LPVOID GetInterface(LPCTSTR pstrName) override;
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		void SetPos(RECT rc, bool bNeedInvalidate /* = true */) override;
		void DoInit() override;
		bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl) override;
		void DoEvent(TEventUI &event) override;

		void SetBkTransparent(bool b);
		bool GetBkTransparent() const;

		void SetResourceResponseCallback(ResourceResponseCallback cb);
		ResourceResponseCallback GetResourceResponseCallback() const;

		void SetAllowProtocols(const std::vector<std::string> vAllowProtocols);
		std::vector<std::string> GetAllowProtocols() const;

		void SetUrl(const CDuiString &url);
		CDuiString GetUrl() const;

		void GoBack();
		void GoForward();
		void Reload();
		void ShowDevTools();
		void CloseDevTools();
		bool CallJavascriptFunction(const CDuiString &strFuncName, const std::vector<VARIANT> &args);
	protected:

	protected:
		bool m_bBkTransparent;
		CDuiString m_strUrl;
		bool m_hCreated;
		ResourceResponseCallback m_ResourceRspCB;
		class CCefUIImpl;
		CCefUIImpl * m_pImpl;
	};
}
#endif
#endif // __UICEF_H__
