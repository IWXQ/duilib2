#ifndef __UIIPADDRESSEX_H__
#define __UIIPADDRESSEX_H__

#pragma once

//���ÿؼ����һ������dtstyle

namespace DuiLib
{
    /// IP����ؼ�
    class UILIB_API CIPAddressUI : public CEditUI
    {
		DECLARE_DUICONTROL(CIPAddressUI)
    public:
        CIPAddressUI();
        LPCTSTR GetClass() const;
        LPVOID GetInterface(LPCTSTR pstrName);
        UINT GetControlFlags() const;
        void DoEvent(TEventUI& event);
        void PaintText(HDC hDC);

        void SetIP(LPCWSTR lpIP);
        base::String GetIP();

    private:
        void CharToInt();
        void GetNumInput(TCHAR chKey);
        void UpdateText();
        void IncNum();
        void DecNum();

    protected:
        int m_nFirst;
        int m_nSecond;
        int m_nThird;
        int m_nFourth;
        int m_nActiveSection;

        TCHAR m_chNum;
        base::String m_strNum;
    };
}
#endif // __UIIPADDRESSEX_H__