#include "stdafx.h"
#include "Windows/PipeHelper.h"
#include "Windows/PipeClient.h"
#include "Common/Buffer.h"
#include "Log/LogEx.h"
#include <stdint.h>

extern uint32_t SuperFastHash(const char * data, int len, int nStep);

CPipeClient::CPipeClient(TCHAR *pipename, DWORD timeout) : CCommunication()
{
    L_INFO(_T("CPipeClient::CPipeClient %X \r\n"), this);
#ifdef UNICODE
    m_szPipeName = wcsdup(pipename);
#else
    m_szPipeName = strdup(pipename);
#endif
    m_dwTimeout = timeout;
    m_hPipe = INVALID_HANDLE_VALUE;
    m_ParamMap.clear();

    InitializeCriticalSection(&m_csParamLock);
}

CPipeClient::~CPipeClient()
{
    std::map<UINT32, CBaseObject*>::iterator Itor;
    std::list<CBaseObject*> TmpList;
    std::list<CBaseObject*>::iterator TmpListItor;

    L_INFO(_T("CPipeClient::~CPipeClient %X \r\n"), this);
    free(m_szPipeName);

    EnterCriticalSection(&m_csParamLock);
    for (Itor = m_ParamMap.begin(); Itor != m_ParamMap.end(); Itor++)
    {
        if (Itor->second)
        {
            TmpList.push_back(Itor->second);
        }
    }
    m_ParamMap.clear();
    LeaveCriticalSection(&m_csParamLock);

    for (TmpListItor = TmpList.begin(); TmpListItor != TmpList.end(); TmpListItor++)
    {
        (*TmpListItor)->Release();
    }

    TmpList.clear();

    DeleteCriticalSection(&m_csParamLock);

    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        DisconnectNamedPipe(m_hPipe);
        CloseHandle(m_hPipe);
        m_hPipe = INVALID_HANDLE_VALUE;
    }
}

HANDLE CPipeClient::InitSyncLock()
{
    TCHAR szSyncEventName[MAX_PATH] = { 0 };
    int nPipeNameLen = _tcslen(m_szPipeName);
    TCHAR* ptr = m_szPipeName + nPipeNameLen;
    while (*ptr != _T('\\') && ptr != m_szPipeName)
    {
        ptr--;
    }
    if (*ptr == _T('\\'))
    {
        ptr++;
    }

    _stprintf(szSyncEventName, _T("Global\\%s_Locker"), ptr);

    HANDLE hSyncEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, szSyncEventName);

    return hSyncEvent;
}

BOOL CPipeClient::Connect()
{
    int retry = 0;

    SetLastError(ERROR_SUCCESS);
    //防止重复连接
    if (IsConnected())
    {
        return FALSE;
    }

connect:
    HANDLE hSyncEvent = InitSyncLock();
    if (hSyncEvent)
    {
        DWORD dwRet = WaitForSingleObject(hSyncEvent, m_dwTimeout);
        if (dwRet == WAIT_OBJECT_0)
        {
            m_hPipe = PipeConnect(m_szPipeName, m_dwTimeout);
        }
        else
        {
            m_hPipe = INVALID_HANDLE_VALUE;
        }
        CloseHandle(hSyncEvent);
    }
    else
    {
        m_hPipe = PipeConnect(m_szPipeName, m_dwTimeout);
    }

    if (m_hPipe != INVALID_HANDLE_VALUE)
    {
        RegisterEndHandle(CPipeClient::PipeClear);
        StartCommunication();

        return TRUE;
    }
    else
    {
        if (GetLastError() == ERROR_PIPE_BUSY)
        {
            retry++;
            if (retry < 3)
            {
                goto connect;
            }
        }
    }

    return FALSE;
}

void CPipeClient::DisConnect()
{
    AddRef();
    StopCommunication();

    Release();
}

BOOL CPipeClient::IsConnected()
{
    return (m_hPipe != INVALID_HANDLE_VALUE);
}

void CPipeClient::PipeClear(ICommunication* param)
{
    CPipeClient *Pipe = dynamic_cast<CPipeClient *>(param);

    if (Pipe)
    {
        Pipe->StopCommunication();
    }
}

IPacketBuffer* CPipeClient::RecvAPacket(HANDLE StopEvent)
{
    IPacketBuffer* Buffer;
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        SetLastError(ERROR_BUSY);
        return NULL;
    }

    Buffer = PipeRecvAPacket(m_hPipe, INFINITE, StopEvent);

    return Buffer;
}

BOOL CPipeClient::SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent)
{
    BOOL ReturnValue;
    SetLastError(ERROR_SUCCESS);

    if (!IsConnected())
    {
        SetLastError(ERROR_BUSY);
        return FALSE;
    }

    ReturnValue = PipeWriteNBytes(m_hPipe, Buffer->GetData(), Buffer->GetBufferLength(), INFINITE, StopEvent);

    return ReturnValue;
}

CBaseObject* CPipeClient::GetParam(const CHAR* ParamKeyword)
{
    CBaseObject* pRet = NULL;
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
    std::map<UINT32, CBaseObject*>::iterator Itor;

    EnterCriticalSection(&m_csParamLock);

    Itor = m_ParamMap.find(uHash);
    if (Itor != m_ParamMap.end())
    {
        pRet = Itor->second;
        if (pRet)
        {
            pRet->AddRef();
        }
    }

    LeaveCriticalSection(&m_csParamLock);

    return pRet;
}

VOID CPipeClient::SetParam(const CHAR* ParamKeyword, CBaseObject* Param)
{
    std::map<UINT32, CBaseObject*>::iterator Itor;
    CBaseObject* OldParam = NULL;
    UINT32 uHash = SuperFastHash(ParamKeyword, strlen(ParamKeyword), 1);
    if (Param)
    {
        Param->AddRef();
    }

    EnterCriticalSection(&m_csParamLock);

    Itor = m_ParamMap.find(uHash);
    if (Itor != m_ParamMap.end())
    {
        OldParam = Itor->second;
    }

    m_ParamMap[uHash] = Param;

    LeaveCriticalSection(&m_csParamLock);

    if (OldParam)
    {
        OldParam->Release();
    }

    return;
}