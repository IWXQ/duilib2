#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once
#include "OAIdl.h"
#include <string>
#include <vector>
#include <map>

namespace DuiLib {

    class UILIB_API STRINGorID {
      public:
        STRINGorID(LPCTSTR lpString) : m_lpstr(lpString) { }
        STRINGorID(UINT nID) : m_lpstr(MAKEINTRESOURCE(nID)) { }
        LPCTSTR m_lpstr;
    };

    class UILIB_API CDuiPoint : public tagPOINT {
      public:
        CDuiPoint();
        CDuiPoint(const POINT &src);
        CDuiPoint(int x, int y);
        CDuiPoint(LPARAM lParam);
    };

    class UILIB_API CDuiSize : public tagSIZE {
      public:
        CDuiSize();
        CDuiSize(const SIZE &src);
        CDuiSize(const RECT rc);
        CDuiSize(int cx, int cy);
    };

    class UILIB_API CDuiRect : public tagRECT {
      public:
        CDuiRect();
        CDuiRect(const RECT &src);
        CDuiRect(int iLeft, int iTop, int iRight, int iBottom);

        int GetWidth() const;
        int GetHeight() const;
        void Empty();
        bool IsNull() const;
        void Join(const RECT &rc);
        void ResetOffset();
        void Normalize();
        void Offset(int cx, int cy);
        void Inflate(int cx, int cy);
        void Deflate(int cx, int cy);
        void Union(CDuiRect &rc);
    };

    class UILIB_API CStdPtrArray {
      public:
        CStdPtrArray(int iPreallocSize = 0);
        CStdPtrArray(const CStdPtrArray &src);
        ~CStdPtrArray();

        void Empty();
        void Resize(int iSize);
        bool IsEmpty() const;
        int Find(LPVOID iIndex) const;
        bool Add(LPVOID pData);
        bool SetAt(int iIndex, LPVOID pData);
        bool InsertAt(int iIndex, LPVOID pData);
        bool Remove(int iIndex);
        int GetSize() const;
        LPVOID *GetData();

        LPVOID GetAt(int iIndex) const;
        LPVOID operator[] (int nIndex) const;

      protected:
        LPVOID *m_ppVoid;
        int m_nCount;
        int m_nAllocated;
    };

	class UILIB_API CDuiString
	{
	public:
		enum { MAX_LOCAL_STRING_LEN = 63 };

		CDuiString();
		CDuiString(const TCHAR ch);
		CDuiString(const CDuiString& src);
		CDuiString(LPCTSTR lpsz, int nLen = -1);
		~CDuiString();
		CDuiString ToString();

		void Empty();
		int GetLength() const;
		bool IsEmpty() const;
		TCHAR GetAt(int nIndex) const;
		void Append(LPCTSTR pstr);
		void Assign(LPCTSTR pstr, int nLength = -1);
		LPCTSTR GetData() const;

		void SetAt(int nIndex, TCHAR ch);
		operator LPCTSTR() const;

		TCHAR operator[] (int nIndex) const;
		const CDuiString& operator=(const CDuiString& src);
		const CDuiString& operator=(const TCHAR ch);
		const CDuiString& operator=(LPCTSTR pstr);
#ifdef _UNICODE
		const CDuiString& CDuiString::operator=(LPCSTR lpStr);
		const CDuiString& CDuiString::operator+=(LPCSTR lpStr);
#else
		const CDuiString& CDuiString::operator=(LPCWSTR lpwStr);
		const CDuiString& CDuiString::operator+=(LPCWSTR lpwStr);
#endif
		CDuiString operator+(const CDuiString& src) const;
		CDuiString operator+(LPCTSTR pstr) const;
		const CDuiString& operator+=(const CDuiString& src);
		const CDuiString& operator+=(LPCTSTR pstr);
		const CDuiString& operator+=(const TCHAR ch);

		bool operator == (LPCTSTR str) const;
		bool operator != (LPCTSTR str) const;
		bool operator <= (LPCTSTR str) const;
		bool operator <  (LPCTSTR str) const;
		bool operator >= (LPCTSTR str) const;
		bool operator >  (LPCTSTR str) const;

		int Compare(LPCTSTR pstr) const;
		int CompareNoCase(LPCTSTR pstr) const;

		void MakeUpper();
		void MakeLower();

		CDuiString Left(int nLength) const;
		CDuiString Mid(int iPos, int nLength = -1) const;
		CDuiString Right(int nLength) const;

		int Find(TCHAR ch, int iPos = 0) const;
		int Find(LPCTSTR pstr, int iPos = 0) const;
		int ReverseFind(TCHAR ch) const;
		int Replace(LPCTSTR pstrFrom, LPCTSTR pstrTo);

		int __cdecl Format(LPCTSTR pstrFormat, ...);
		int __cdecl SmallFormat(LPCTSTR pstrFormat, ...);

	protected:
		LPTSTR m_pstr;
		TCHAR m_szBuffer[MAX_LOCAL_STRING_LEN + 1];
	};
  
    UILIB_API bool StringIsInVector(const std::vector<CDuiString> &v, const CDuiString& str, bool bIgnoreCase);

    class UILIB_API CStdStringPtrMap {
      public:
        CStdStringPtrMap();
        ~CStdStringPtrMap();

        LPVOID Find(LPCTSTR key) const;
        bool Insert(LPCTSTR key, LPVOID pData);
        LPVOID Set(LPCTSTR key, LPVOID pData);
        bool Remove(LPCTSTR key);
        void RemoveAll();
        int GetSize() const;
        std::map<CDuiString, LPVOID>::iterator Begin();
        std::map<CDuiString, LPVOID>::iterator End();
      protected:
        std::map<CDuiString, LPVOID> m_Map;
    };

    class UILIB_API CDuiVariant : public VARIANT {
      public:
        CDuiVariant() {
            VariantInit(this);
        }
        CDuiVariant(int i) {
            VariantInit(this);
            this->vt = VT_I4;
            this->intVal = i;
        }
        CDuiVariant(float f) {
            VariantInit(this);
            this->vt = VT_R4;
            this->fltVal = f;
        }
        CDuiVariant(LPOLESTR s) {
            VariantInit(this);
            this->vt = VT_BSTR;
            this->bstrVal = s;
        }
        CDuiVariant(IDispatch *disp) {
            VariantInit(this);
            this->vt = VT_DISPATCH;
            this->pdispVal = disp;
        }

        ~CDuiVariant() {
            VariantClear(this);
        }
    };


	// About code_page, see https://docs.microsoft.com/zh-cn/windows/desktop/Intl/code-page-identifiers
	//
	UILIB_API std::string UnicodeToAnsi(const std::wstring &str, unsigned int code_page = 0);
	UILIB_API std::wstring AnsiToUnicode(const std::string &str, unsigned int code_page = 0);
	UILIB_API std::string UnicodeToUtf8(const std::wstring &str);
	UILIB_API std::wstring Utf8ToUnicode(const std::string &str);
	UILIB_API std::string AnsiToUtf8(const std::string &str, unsigned int code_page = 0);
	UILIB_API std::string Utf8ToAnsi(const std::string &str, unsigned int code_page = 0);

	// BSTR is Unicode, String is Utf8
	UILIB_API std::string BSTRToString(const VARIANT *arg);

	UILIB_API bool IsDevtoolResourceExist();
}// namespace DuiLib

#endif // __UTILS_H__