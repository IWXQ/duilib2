#include "StdAfx.h"
#include "Utils.h"

namespace DuiLib {

    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CDuiPoint::CDuiPoint() {
        x = y = 0;
    }

    CDuiPoint::CDuiPoint(const POINT &src) {
        x = src.x;
        y = src.y;
    }

    CDuiPoint::CDuiPoint(int _x, int _y) {
        x = _x;
        y = _y;
    }

    CDuiPoint::CDuiPoint(LPARAM lParam) {
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
    }


    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CDuiSize::CDuiSize() {
        cx = cy = 0;
    }

    CDuiSize::CDuiSize(const SIZE &src) {
        cx = src.cx;
        cy = src.cy;
    }

    CDuiSize::CDuiSize(const RECT rc) {
        cx = rc.right - rc.left;
        cy = rc.bottom - rc.top;
    }

    CDuiSize::CDuiSize(int _cx, int _cy) {
        cx = _cx;
        cy = _cy;
    }


    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CDuiRect::CDuiRect() {
        left = top = right = bottom = 0;
    }

    CDuiRect::CDuiRect(const RECT &src) {
        left = src.left;
        top = src.top;
        right = src.right;
        bottom = src.bottom;
    }

    CDuiRect::CDuiRect(int iLeft, int iTop, int iRight, int iBottom) {
        left = iLeft;
        top = iTop;
        right = iRight;
        bottom = iBottom;
    }

    int CDuiRect::GetWidth() const {
        return right - left;
    }

    int CDuiRect::GetHeight() const {
        return bottom - top;
    }

    void CDuiRect::Empty() {
        left = top = right = bottom = 0;
    }

    bool CDuiRect::IsNull() const {
        return (left == 0 && right == 0 && top == 0 && bottom == 0);
    }

    void CDuiRect::Join(const RECT &rc) {
        if( rc.left < left ) left = rc.left;

        if( rc.top < top ) top = rc.top;

        if( rc.right > right ) right = rc.right;

        if( rc.bottom > bottom ) bottom = rc.bottom;
    }

    void CDuiRect::ResetOffset() {
        ::OffsetRect(this, -left, -top);
    }

    void CDuiRect::Normalize() {
        if( left > right ) {
            int iTemp = left;
            left = right;
            right = iTemp;
        }

        if( top > bottom ) {
            int iTemp = top;
            top = bottom;
            bottom = iTemp;
        }
    }

    void CDuiRect::Offset(int cx, int cy) {
        ::OffsetRect(this, cx, cy);
    }

    void CDuiRect::Inflate(int cx, int cy) {
        ::InflateRect(this, cx, cy);
    }

    void CDuiRect::Deflate(int cx, int cy) {
        ::InflateRect(this, -cx, -cy);
    }

    void CDuiRect::Union(CDuiRect &rc) {
        ::UnionRect(this, this, &rc);
    }


    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    CStdPtrArray::CStdPtrArray(int iPreallocSize) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(iPreallocSize) {
        ASSERT(iPreallocSize >= 0);

        if( iPreallocSize > 0 ) 
			m_ppVoid = static_cast<LPVOID *>(malloc(iPreallocSize * sizeof(LPVOID)));
    }

    CStdPtrArray::CStdPtrArray(const CStdPtrArray &src) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(0) {
        for(int i = 0; i < src.GetSize(); i++)
            Add(src.GetAt(i));
    }

    CStdPtrArray::~CStdPtrArray() {
        if( m_ppVoid != NULL ) 
			free(m_ppVoid);
    }

    void CStdPtrArray::Empty() {
        if( m_ppVoid != NULL ) 
			free(m_ppVoid);

        m_ppVoid = NULL;
        m_nCount = m_nAllocated = 0;
    }

    void CStdPtrArray::Resize(int iSize) {
        Empty();
        m_ppVoid = static_cast<LPVOID *>(malloc(iSize * sizeof(LPVOID)));
        ::ZeroMemory(m_ppVoid, iSize * sizeof(LPVOID));
        m_nAllocated = iSize;
        m_nCount = iSize;
    }

    bool CStdPtrArray::IsEmpty() const {
        return m_nCount == 0;
    }

    bool CStdPtrArray::Add(LPVOID pData) {
        if( ++m_nCount >= m_nAllocated) {
            int nAllocated = m_nAllocated * 2;

            if( nAllocated == 0 ) 
				nAllocated = 11;

            LPVOID *ppVoid = static_cast<LPVOID *>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));

            if( ppVoid != NULL ) {
                m_nAllocated = nAllocated;
                m_ppVoid = ppVoid;
            } else {
                --m_nCount;
                return false;
            }
        }

        m_ppVoid[m_nCount - 1] = pData;
        return true;
    }

    bool CStdPtrArray::InsertAt(int iIndex, LPVOID pData) {
        if( iIndex == m_nCount ) 
			return Add(pData);

        if( iIndex < 0 || iIndex > m_nCount ) 
			return false;

        if( ++m_nCount >= m_nAllocated) {
            int nAllocated = m_nAllocated * 2;

            if( nAllocated == 0 ) 
				nAllocated = 11;

            LPVOID *ppVoid = static_cast<LPVOID *>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));

            if( ppVoid != NULL ) {
                m_nAllocated = nAllocated;
                m_ppVoid = ppVoid;
            } else {
                --m_nCount;
                return false;
            }
        }

        memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(LPVOID));
        m_ppVoid[iIndex] = pData;
        return true;
    }

    bool CStdPtrArray::SetAt(int iIndex, LPVOID pData) {
        if( iIndex < 0 || iIndex >= m_nCount ) 
			return false;

        m_ppVoid[iIndex] = pData;
        return true;
    }

    bool CStdPtrArray::Remove(int iIndex) {
        if( iIndex < 0 || iIndex >= m_nCount ) 
			return false;

        if( iIndex < --m_nCount ) 
			::CopyMemory(m_ppVoid + iIndex, m_ppVoid + iIndex + 1, (m_nCount - iIndex) * sizeof(LPVOID));

        return true;
    }

    int CStdPtrArray::Find(LPVOID pData) const {
        for( int i = 0; i < m_nCount; i++ ) 
			if( m_ppVoid[i] == pData ) 
				return i;

        return -1;
    }

    int CStdPtrArray::GetSize() const {
        return m_nCount;
    }

    LPVOID *CStdPtrArray::GetData() {
        return m_ppVoid;
    }

    LPVOID CStdPtrArray::GetAt(int iIndex) const {
        if( iIndex < 0 || iIndex >= m_nCount ) 
			return NULL;

        return m_ppVoid[iIndex];
    }

    LPVOID CStdPtrArray::operator[] (int iIndex) const {
        ASSERT(iIndex >= 0 && iIndex < m_nCount);
        return m_ppVoid[iIndex];
    }


   



    /////////////////////////////////////////////////////////////////////////////
    //
    //
    CStdStringPtrMap::CStdStringPtrMap() {
    }

    CStdStringPtrMap::~CStdStringPtrMap() {
    }

    void CStdStringPtrMap::RemoveAll() {
		m_Map.clear();
    }

    LPVOID CStdStringPtrMap::Find(LPCTSTR key) const {
		std::map<ppx::base::String, LPVOID>::const_iterator it = m_Map.find(key);
		if (it == m_Map.end())
			return NULL;
		const LPVOID value = m_Map.at(key);
		return value;
    }

    bool CStdStringPtrMap::Insert(LPCTSTR key, LPVOID pData) {
		m_Map[key] = pData;
        return true;
    }

    LPVOID CStdStringPtrMap::Set(LPCTSTR key, LPVOID pData) {
		LPVOID old = Find(key);
		m_Map[key] = pData;
        return old;
    }

    bool CStdStringPtrMap::Remove(LPCTSTR key) {
		std::map<ppx::base::String, LPVOID>::const_iterator it = m_Map.find(key);
		if (it == m_Map.end())
			return false;

		m_Map.erase(key);

        return true;
    }

    int CStdStringPtrMap::GetSize() const {
        return m_Map.size();
    }


	std::map<ppx::base::String, LPVOID>::iterator CStdStringPtrMap::Begin() {
		return m_Map.begin();
	}

	std::map<ppx::base::String, LPVOID>::iterator CStdStringPtrMap::End() {
		return m_Map.end();
	}

    bool StringIsInVector(const std::vector<ppx::base::String> &v, const ppx::base::String& str, bool bIgnoreCase) {
        for (size_t i = 0; i < v.size(); i++) {
            if (bIgnoreCase) {
                if (v[i].CompareNoCase(str.GetData()) == 0) {
                    return true;
                }
            }
            else {
                if (v[i].Compare(str.GetData()) == 0) {
                    return true;
                }
            }
        }
        return false;
    }
} // namespace DuiLib
