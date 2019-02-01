#ifndef __UICEF_H__
#define __UICEF_H__
#pragma once

namespace DuiLib {

	class UILIB_API CCefUI : public CControlUI {
		DECLARE_DUICONTROL(CCefUI)
	public:
		CCefUI();
		virtual ~CCefUI();

		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) override;

		BOOL LoadUrl(const base::String &url);
		void ReLoad();

		BOOL CanGoForword();
		void GoForword();

		BOOL CanGoBack();
		void GoBack();

		base::String GetUrl() const;

		void ClearHistory();

		void SetErrorPageUrl(const base::String &url);
		base::String GetErrorPageUrl() const;

		void SetShowDevTool(bool bShow);
	protected:
		bool m_bShowDevTool;
		base::String m_strUrl;
		base::String m_strErrorPageUrl;
		static bool m_bInit;
	};
}

#endif // !__UICEF_H__