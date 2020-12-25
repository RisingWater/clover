#include "stdafx.h"
#include "TCPBase.h"
#include "TCPCommon.h"
#include "Common/IBuffer.h"

static IPacketBuffer* GetPacketBufferFromDataStream(HANDLE hDataBuffer)
{
    IPacketBuffer* pPacket = NULL;
    DWORD dwReaded = 0;
    DWORD dwCurrentSize = DataStreamBufferGetCurrentDataSize(hDataBuffer);
    if (dwCurrentSize >= BASE_PACKET_HEADER_LEN)
    {
        BASE_PACKET_T header;

        DataStreamBufferGetData(hDataBuffer, (BYTE*)&header, BASE_PACKET_HEADER_LEN, &dwReaded);
        DataStreamBufferAddDataFront(hDataBuffer, (BYTE*)&header, dwReaded);

        if (dwReaded == BASE_PACKET_HEADER_LEN)
        {
            if (header.Length <= dwCurrentSize)
            {
                pPacket = CreateIBufferInstance(header.Length);
                DataStreamBufferGetData(hDataBuffer, pPacket->GetData(), header.Length, &dwReaded);

                if (dwReaded != header.Length)
                {
                    DataStreamBufferAddDataFront(hDataBuffer, (BYTE*)pPacket, dwReaded);
                    pPacket->Release();
                    pPacket = NULL;
                }
            }
        }
    }

    return pPacket;
}

CTCPBase::CTCPBase()
{
    m_hDataStreamBuffer = CreateDataStreamBuffer();

    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hSendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_hRecvThread = NULL;
    m_hSendThread = NULL;

    m_pfnRecvFunc = NULL;
    m_pRecvParam = NULL;
    m_pfnEndFunc = NULL;
    m_pEndParam = NULL;

    InitializeCriticalSection(&m_csSendLock);
    InitializeCriticalSection(&m_csFunc);

    m_dwKeepAliveValue = 0;
    InitializeCriticalSection(&m_csLock);
}

CTCPBase::~CTCPBase()
{
    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }
    if (m_hSendEvent)
    {
        CloseHandle(m_hSendEvent);
    }
    if (m_hRecvThread)
    {
        CloseHandle(m_hRecvThread);
    }
    if (m_hSendThread)
    {
        CloseHandle(m_hSendThread);
    }

    if (m_hDataStreamBuffer)
    {
        CloseDataStreamBuffer(m_hDataStreamBuffer);
        m_hDataStreamBuffer = NULL;
    }

    DeleteCriticalSection(&m_csSendLock);
    DeleteCriticalSection(&m_csFunc);
    DeleteCriticalSection(&m_csLock);
}

BOOL CTCPBase::InitBase()
{
    int flag = 1;
    setsockopt(m_hSock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

#ifdef WIN32
    flag = 1;
    ioctlsocket(m_hSock, FIONBIO, (unsigned long *)&flag);
#else
    ioctl(m_socket, FIONBIO, 0);
#endif

    AddRef();
    m_hRecvThread = CreateThread(NULL, 0, CTCPBase::RecvProc, this, 0, NULL);

    AddRef();
    m_hSendThread = CreateThread(NULL, 0, CTCPBase::SendProc, this, 0, NULL);

    return TRUE;
}

VOID CTCPBase::DoneBase()
{
    SetEvent(m_hStopEvent);
}

VOID CTCPBase::SendPacket(IPacketBuffer* Packet)
{
    Packet->AddRef();
    EnterCriticalSection(&m_csSendLock);
    m_SendList.push_back(Packet);
    SetEvent(m_hSendEvent);
    LeaveCriticalSection(&m_csSendLock);
}

VOID CTCPBase::RegisterRecvProcess(_TCPRecvPacketProcess Process, CBaseObject* Param)
{
    CBaseObject* pOldParam = NULL;

    EnterCriticalSection(&m_csFunc);
    m_pfnRecvFunc = Process;

    pOldParam = m_pRecvParam;
    m_pRecvParam = Param;
    if (m_pRecvParam)
    {
        m_pRecvParam->AddRef();
    }
    LeaveCriticalSection(&m_csFunc);

    if (pOldParam)
    {
        pOldParam->Release();
    }
}

VOID CTCPBase::RegisterEndProcess(_TCPEndProcess Process, CBaseObject* Param)
{
    CBaseObject* pOldParam = NULL;

    EnterCriticalSection(&m_csFunc);
    m_pfnEndFunc = Process;

    pOldParam = m_pEndParam;
    m_pEndParam = Param;
    if (m_pEndParam)
    {
        m_pEndParam->AddRef();
    }
    LeaveCriticalSection(&m_csFunc);

    if (pOldParam)
    {
        pOldParam->Release();
    }
}

DWORD WINAPI CTCPBase::RecvProc(void* pParam)
{
    CTCPBase* tcp = (CTCPBase*)pParam;
    while (TRUE)
    {
        if (!tcp->RecvProcess(tcp->m_hStopEvent))
        {
            break;
        }
    }

    EnterCriticalSection(&tcp->m_csFunc);
    if (tcp->m_pfnEndFunc)
    {
        tcp->m_pfnEndFunc(tcp, tcp->m_pEndParam);
    }
    LeaveCriticalSection(&tcp->m_csFunc);

    L_INFO(_T("CTCPBase: Recv Thread Stop\r\n"));

    SetEvent(tcp->m_hStopEvent);

    tcp->Release();

    return 0;
}

DWORD WINAPI CTCPBase::SendProc(void* pParam)
{
    CTCPBase* tcp = (CTCPBase*)pParam;
    while (TRUE)
    {
        if (!tcp->SendProcess(tcp->m_hStopEvent))
        {
            break;
        }
    }

    L_INFO(_T("CTCPBase: Send Thread Stop\r\n"));

    tcp->Release();

    return 0;
}

BOOL CTCPBase::SendProcess(HANDLE StopEvent)
{
    BOOL SendOK = TRUE;
    HANDLE h[2] = {
        m_hSendEvent,
        m_hStopEvent,
    };

    DWORD Ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);
    if (Ret != WAIT_OBJECT_0)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csSendLock);

    if (!m_SendList.empty())
    {
        IPacketBuffer* Packet = m_SendList.front();
        m_SendList.pop_front();

        LeaveCriticalSection(&m_csSendLock);

        DWORD Length = Packet->GetBufferLength();
        PBYTE Data = Packet->GetData();

        while (Length > 0)
        {
            DWORD sendSize = 0;
            BOOL res = TCPSocketWrite(m_hSock, Data, Length, &sendSize, StopEvent);
            if (res)
            {
                Data += sendSize;
                Length -= sendSize;
            }
            else
            {
                SendOK = FALSE;
                break;
            }
        }

        Packet->Release();
    }
    else
    {
        ResetEvent(m_hSendEvent);
        LeaveCriticalSection(&m_csSendLock);
    }

    return SendOK;
}

BOOL CTCPBase::RecvProcess(HANDLE StopEvent)
{
    BOOL Ret = FALSE;

    BYTE Buffer[4096];
    IPacketBuffer* Packet = NULL;

    DWORD recvSize;
    BOOL res = TCPSocketRead(m_hSock, (BYTE*)Buffer, 4096, &recvSize, StopEvent);
    if (!res)
    {
        return FALSE;
    }

    DataStreamBufferAddData(m_hDataStreamBuffer, Buffer, recvSize);
    while (TRUE)
    {
        Packet = GetPacketBufferFromDataStream(m_hDataStreamBuffer);
        if (Packet != NULL)
        {
            EnterCriticalSection(&m_csFunc);
            if (m_pfnRecvFunc)
            {
                Ret = m_pfnRecvFunc(Packet, this, m_pRecvParam);
            }
            LeaveCriticalSection(&m_csFunc);

            Packet->Release();
        }
        else
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID CTCPBase::GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort)
{
    strncpy(SrcAddress, m_szSrcAddress, BufferLen);

    *SrcPort = m_dwSrcPort;

    return;
}

VOID CTCPBase::GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort)
{
    strncpy(DstAddress, m_szDstAddress, BufferLen);

    *DstPort = m_dwDstPort;

    return;
}

void CTCPBase::KeepAliveValueClear()
{
    EnterCriticalSection(&m_csLock);
    m_dwKeepAliveValue = 0;
    LeaveCriticalSection(&m_csLock);
}

DWORD CTCPBase::AddKeepAliveValue()
{
    DWORD Ret;

    EnterCriticalSection(&m_csLock);

    m_dwKeepAliveValue++;
    Ret = m_dwKeepAliveValue;

    LeaveCriticalSection(&m_csLock);

    return Ret;
}