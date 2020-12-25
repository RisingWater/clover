#pragma once

#ifdef WIN32
#include <Windows.h>
#include <list>
#else
#include <winpr/wtypes.h>
#include <winpr/synch.h>
#endif

#include "DllExport.h"
#include "Base/BaseObject.h"

class DLL_COMMONLIB_API IShareMemory : public virtual CBaseObject
{
public:
    virtual BOOL CreateShareMemory(DWORD nSize) = 0;

    virtual BOOL OpenShareMemory(BOOL bReadOnly = TRUE) = 0;

    virtual int ReadData(BYTE* pBuffer, int nBufferSize) = 0;

    virtual int WriteData(BYTE* pBuffer, int nBufferSize) = 0;

    virtual int GetDataSize() = 0;

    virtual void Close() = 0;
};

extern "C"
{
    DLL_COMMONLIB_API IShareMemory* WINAPI CreateIShareMemoryInstance(BOOL bLocal, TCHAR* pFileName);
}
