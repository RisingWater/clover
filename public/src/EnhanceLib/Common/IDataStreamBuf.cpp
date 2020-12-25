#include "stdafx.h"
#include "Common/IDataStreamBuf.h"
#include "Common/DataStreamBuf.h"

HANDLE WINAPI CreateDataStreamBuffer()
{
    CDataStreamBuf* pBuffer = new CDataStreamBuf();

    return (HANDLE)pBuffer;
}

void WINAPI DataStreamBufferClearAllData(HANDLE hBuffer)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
    pBuffer->ClearAll();
}

void WINAPI DataStreamBufferAddData(HANDLE hBuffer, BYTE *pData, DWORD dwSize)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
    pBuffer->AddData(pData, dwSize);
}

void WINAPI DataStreamBufferAddDataFront(HANDLE hBuffer, BYTE *pData, DWORD dwSize)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
    pBuffer->AddDataFront(pData, dwSize);
}

BYTE* WINAPI DataStreamBufferGetAllData(HANDLE hBuffer, DWORD *lpDataOut)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
    return pBuffer->GetAllData(lpDataOut);
}

BOOL WINAPI DataStreamBufferGetData(HANDLE hBuffer, BYTE *pBuf, DWORD dwBufSize, DWORD *lpNumOfGet)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
    return pBuffer->GetData(pBuf, dwBufSize, lpNumOfGet);
}

DWORD WINAPI DataStreamBufferGetCurrentDataSize(HANDLE hBuffer)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;

    return pBuffer->GetCurrentDataSize();
}

void WINAPI CloseDataStreamBuffer(HANDLE hBuffer)
{
    CDataStreamBuf* pBuffer = (CDataStreamBuf*)hBuffer;
    delete pBuffer;
}

void WINAPI GetGuidText(GUID guid, TCHAR* pBuf, DWORD dwBufSize)
{
    _stprintf_s(pBuf, dwBufSize, _T("{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"), guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}