#include "stdafx.h"
#include <Windows/ShareMemory.h>
#include "Log/LogEx.h"

CShareMemory::CShareMemory(BOOL bLocal, TCHAR* pFileName)
{
    if (bLocal)
    {
        m_bGlobal = FALSE;
        _stprintf_s(m_szFileName, _T("Local\\%s"), pFileName);
    }
    else
    {
        m_bGlobal = TRUE;
        _stprintf_s(m_szFileName, _T("Global\\%s"), pFileName);
    }

    m_pDataStart = NULL;
    m_pShareMemory = NULL;
    m_dwTotalSize = 0;
    m_dwDataSize = 0;
}

CShareMemory::~CShareMemory()
{
    Close();
}

BOOL CShareMemory::CreateShareMemory(DWORD nSize)
{
    BOOL bRet = FALSE;
    do
    {
        m_dwTotalSize = sizeof(DWORD) + nSize;

        if (m_bGlobal)
        {
            SECURITY_ATTRIBUTES sa;
            SECURITY_DESCRIPTOR sd;
            InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
            SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = FALSE;
            sa.lpSecurityDescriptor = &sd;
            m_hFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, m_dwTotalSize, m_szFileName);
        }
        else
        {
            m_hFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m_dwTotalSize, m_szFileName);
        }

        if (m_hFileHandle == NULL)
        {
            L_ERROR(_T("CreateFileMapping fail %d \r\n"), GetLastError());
            break;
        }

        m_pShareMemory = (BYTE*)MapViewOfFile(m_hFileHandle,   // handle to map object
            FILE_MAP_ALL_ACCESS, // read/write permission
            0,
            0,
            m_dwTotalSize);

        if (m_pShareMemory == NULL)
        {
            L_ERROR(_T("MapViewOfFile %s fail %d \r\n"), GetLastError());
            break;
        }

        ZeroMemory(m_pShareMemory, m_dwTotalSize);
        memcpy(m_pShareMemory, &nSize, sizeof(DWORD));
        m_pDataStart = m_pShareMemory + sizeof(DWORD);
        m_dwDataSize = nSize;
        bRet = TRUE;
    } while (FALSE);

    if (bRet == FALSE)
    {
        Close();
    }
    return bRet;
}

void CShareMemory::Close()
{
    if (m_pShareMemory)
    {
        UnmapViewOfFile(m_pShareMemory);
        m_pShareMemory = NULL;
    }
    if (m_hFileHandle)
    {
        CloseHandle(m_hFileHandle);
        m_hFileHandle = NULL;
    }

    m_pDataStart = NULL;
    m_pShareMemory = NULL;
    m_dwTotalSize = 0;
    m_dwDataSize = 0;
}

BOOL CShareMemory::OpenShareMemory(BOOL bReadOnly /* = TRUE */)
{
    DWORD dwDesiredAccess;
    BOOL bRet = FALSE;
    do
    {
        if (bReadOnly)
        {
            dwDesiredAccess = FILE_MAP_READ;
        }
        else
        {
            dwDesiredAccess = FILE_MAP_ALL_ACCESS;
        }
        m_hFileHandle = OpenFileMapping(
            FILE_MAP_ALL_ACCESS,   // read/write access
            FALSE,                 // do not inherit the name
            m_szFileName);               // name of mapping object
        //L_INFO(_T("OpenFileMapping m_hFileHandle %X \r\n"), m_hFileHandle);
        if (m_hFileHandle == NULL)
        {
            L_ERROR(_T("OpenFileMapping %s fail %d \r\n"), m_szFileName, GetLastError());
            break;
        }

        //L_INFO(_T("start MapViewOfFile --- \r\n"));
        m_pShareMemory = (BYTE*)MapViewOfFile(m_hFileHandle,   // handle to map object
            FILE_MAP_ALL_ACCESS, // read/write permission
            0,
            0,
            m_dwTotalSize);
        //L_INFO(_T("m_pShareMemory %X  --- \r\n"), m_pShareMemory);
        if (m_pShareMemory == NULL)
        {
            L_ERROR(_T("MapViewOfFile %s fail %d \r\n"), m_szFileName, GetLastError());
            break;
        }

        memcpy(&m_dwDataSize, m_pShareMemory, sizeof(DWORD));
        //L_INFO(_T("m_dwDataSize %d \r\n"), m_dwDataSize);
        m_dwTotalSize = m_dwDataSize + sizeof(DWORD);
        m_pDataStart = m_pShareMemory + sizeof(DWORD);

        //L_INFO(_T("m_pDataStart %X \r\n"), m_pDataStart);
        bRet = TRUE;
    } while (FALSE);

    if (bRet == FALSE)
    {
        Close();
    }

    return bRet;
}

int CShareMemory::WriteData(BYTE* pBuffer, int nBufferSize)
{
    if ((DWORD)nBufferSize <= m_dwDataSize)
    {
        memcpy(m_pDataStart, pBuffer, nBufferSize);
        return nBufferSize;
    }
    return 0;
}

int CShareMemory::ReadData(BYTE* pBuffer, int nBufferSize)
{
    int nMin = min((DWORD)nBufferSize, m_dwDataSize);
    memcpy(pBuffer, m_pDataStart, nMin);
    return nMin;
}

int CShareMemory::GetDataSize()
{
    return m_dwDataSize;
}