#pragma once

#ifdef WIN32
#include <Windows.h>
#include <list>
#else
#include <winpr/wtypes.h>
#include <winpr/tchar.h>
#include <winpr/file.h>
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
    BOOL InitializeShareMemory();

    CHAR m_szLockFileName[MAX_PATH];
    CHAR m_szShareMemoryName[MAX_PATH];
    
    DWORD m_dwSize;
    DWORD m_dwTotalSize;
    BYTE* m_pShareMemory;
    BYTE* m_pShareMemorySize;

    BYTE* m_pDataStart;
    DWORD m_dwDataSize;
	
    int m_iLockId;
    int m_iShareMemoryFd;

    BOOL m_bGlobal;
    BOOL m_bCreate;
};

