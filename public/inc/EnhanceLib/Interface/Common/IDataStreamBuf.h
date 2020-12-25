#ifndef __IDATA_STREAM_BUF_H__
#define __IDATA_STREAM_BUF_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "DllExport.h"

extern "C"
{
	DLL_COMMONLIB_API HANDLE WINAPI CreateDataStreamBuffer();
	DLL_COMMONLIB_API void   WINAPI CloseDataStreamBuffer(HANDLE hBuffer);
	DLL_COMMONLIB_API void   WINAPI DataStreamBufferClearAllData(HANDLE hBuffer);
	DLL_COMMONLIB_API void   WINAPI DataStreamBufferAddData(HANDLE hBuffer, BYTE *pData, DWORD dwSize);
	DLL_COMMONLIB_API void   WINAPI DataStreamBufferAddDataFront(HANDLE hBuffer, BYTE *pData, DWORD dwSize);
	DLL_COMMONLIB_API BYTE*  WINAPI DataStreamBufferGetAllData(HANDLE hBuffer, DWORD *lpDataOut);
	DLL_COMMONLIB_API BOOL   WINAPI DataStreamBufferGetData(HANDLE hBuffer, BYTE *pBuf, DWORD dwBufSize, DWORD *lpNumOfGet);
	DLL_COMMONLIB_API DWORD  WINAPI DataStreamBufferGetCurrentDataSize(HANDLE hBuffer);

	DLL_COMMONLIB_API void   WINAPI GetGuidText(GUID guid, TCHAR* pBuf, DWORD dwBufSize);
}

#endif