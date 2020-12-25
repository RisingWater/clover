#pragma once

#ifdef WIN32
#include <Windows.h>
#include <list>
#else
#include <winpr/wtypes.h>
#include <winpr/synch.h>
#endif

#include "DllExport.h"
#include "Common/IShareMemory.h"

class DLL_COMMONLIB_API CShareMemory : public IShareMemory
{
public:
    CShareMemory(BOOL bLocal, TCHAR* pFileName);
    ~CShareMemory();

    virtual BOOL CreateShareMemory(DWORD nSize);
    virtual BOOL OpenShareMemory(BOOL bReadOnly = TRUE);
    virtual int ReadData(BYTE* pBuffer, int nBufferSize);
    virtual int WriteData(BYTE* pBuffer, int nBufferSize);

    virtual int GetDataSize();
    virtual void Close();

private:
    TCHAR m_szFileName[MAX_PATH];
    LONG m_lRef;

    DWORD m_dwTotalSize;
    BYTE* m_pShareMemory;

    BYTE* m_pDataStart;
    DWORD m_dwDataSize;

    HANDLE m_hFileHandle;

    BOOL m_bGlobal;
};
