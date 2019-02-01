#ifndef __UTILS_H__
#define __UTILS_H__

#pragma once
#include "OAIdl.h"
#include <string>
#include <vector>
#include <map>
#include "base/string.h"

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

  
    UILIB_API bool StringIsInVector(const std::vector<ppx::base::String> &v, const ppx::base::String& str, bool bIgnoreCase);

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
        std::map<ppx::base::String, LPVOID>::iterator Begin();
        std::map<ppx::base::String, LPVOID>::iterator End();
      protected:
        std::map<ppx::base::String, LPVOID> m_Map;
    };

    class CDuiVariant : public VARIANT {
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

}// namespace DuiLib

#endif // __UTILS_H__