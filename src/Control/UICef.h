#ifndef __UICEF_H__
#define __UICEF_H__
#pragma once


namespace DuiLib {
	class UILIB_API CCefUI : public CContainerUI {
		DECLARE_DUICONTROL(CCefUI)
	public:
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
		bool GetBkTransparent();

		void SetUrl(const CDuiString &url);
		CDuiString GetUrl();

		void GoBack();
		void GoForward();
		void Reload();
		void ShowDevTools();
		void CloseDevTools();
	protected:

	protected:
		bool m_bBkTransparent;
		CDuiString m_strUrl;
		bool m_hCreated;
		class CCefUIImpl;
		CCefUIImpl * m_pImpl;
	};
}
#endif // __UICEF_H__
