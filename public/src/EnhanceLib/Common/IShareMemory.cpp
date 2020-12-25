#include "Common/IShareMemory.h"
#ifdef WIN32
#include "Windows/ShareMemory.h"
#else
#include "Linux/ShareMemory.h"
#endif

IShareMemory* WINAPI CreateIShareMemoryInstance(BOOL bLocal, TCHAR* pFileName)
{
    return new CShareMemory(bLocal, pFileName);
}