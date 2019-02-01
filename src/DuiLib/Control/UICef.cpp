#include "StdAfx.h"

namespace DuiLib {
	IMPLEMENT_DUICONTROL(CCefUI)

	bool CCefUI::m_bInit = false;

	CCefUI::CCefUI() : m_bShowDevTool(false) {
		PPX_ASSERT(m_bInit);
	}

	CCefUI::~CCefUI() {

	}

	void CCefUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue) {
		if (_tcscmp(pstrName, _T("url")) == 0) {
			m_strUrl = pstrValue;
		}
		else if (_tcscmp(pstrName, _T("errorpageurl")) == 0) {
			m_strErrorPageUrl = pstrValue;
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	BOOL CCefUI::LoadUrl(const base::String &url) {
		return FALSE;
	}

	void CCefUI::ReLoad() {

	}

	BOOL CCefUI::CanGoForword() {
		return FALSE;
	}

	void CCefUI::GoForword() {

	}

	BOOL CCefUI::CanGoBack() {
		return FALSE;
	}

	void CCefUI::GoBack() {

	}

	base::String CCefUI::GetUrl() const {
		return m_strUrl;
	}

	void CCefUI::ClearHistory() {

	}

	void CCefUI::SetErrorPageUrl(const base::String &url) {

	}

	base::String CCefUI::GetErrorPageUrl() const {
		return m_strErrorPageUrl;
	}

	void CCefUI::SetShowDevTool(bool bShow) {
		m_bShowDevTool = bShow;
	}

}