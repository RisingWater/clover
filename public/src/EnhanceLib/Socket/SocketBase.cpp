#include "stdafx.h"
#include "SocketBase.h"
#include "SocketHelper.h"

CSocketBase::CSocketBase(SocketType Type) : CCommunication()
{
    ZeroMemory(m_szDstAddress, 128);
    m_dwDstPort = 0;

    ZeroMemory(m_szSrcAddress, 128);
    m_dwSrcPort = 0;

    m_Cache = new CCache(NULL);
    m_bAlive = FALSE;

    m_eSocketType = Type;

    m_socket = INVALID_SOCKET;

    InitializeCriticalSection(&m_csParamLock);
}

CSocketBase::~CSocketBase()
{
    m_Cache->Release();

    EnterCriticalSection(&m_csParamLock);

    m_ParamMap.clear();

    LeaveCriticalSection(&m_csParamLock);

    DeleteCriticalSection(&m_csParamLock);
}

BOOL WINAPI CSocketBase::Open()
{
    m_bAlive = TRUE;

    if (m_eSocketType == NET_SOCKET)
    {
        //set tcp no delay
        int flag = 1;
        setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

        //set non block mode
#ifdef WIN32
        flag = 1;
        ioctlsocket(m_socket, FIONBIO, (unsigned long *)&flag);
#else
        ioctl(m_socket, FIONBIO, 0);
#endif
    }

    RegisterEndHandle(CSocketBase::SocketClear);
    StartCommunication();

    return TRUE;
}

void WINAPI CSocketBase::Close()
{
    m_bAlive = FALSE;
    StopCommunication();

    if (m_socket != INVALID_SOCKET)
    {
        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

PVOID CSocketBase::GetParam(const CHAR* ParamKeyword)
{
    PVOID pRet = NULL;
    std::map<std::string, PVOID>::iterator Itor;

    EnterCriticalSection(&m_csParamLock);

    Itor = m_ParamMap.find(ParamKeyword);
    if (Itor != m_ParamMap.end())
    {
        pRet = Itor->second;
    }

    LeaveCriticalSection(&m_csParamLock);

    return pRet;
}

VOID CSocketBase::SetParam(const CHAR* ParamKeyword, PVOID Param)
{
    EnterCriticalSection(&m_csParamLock);

    m_ParamMap[ParamKeyword] = Param;

    LeaveCriticalSection(&m_csParamLock);

    return;
}

IPacketBuffer* CSocketBase::RecvAPacket(HANDLE StopEvent)
{
    do
    {
        //read cache first
        BASE_PACKET_T* Packet;
        Packet = m_Cache->GetPacket();
        //if cache has a packet, return packet
        if (Packet != NULL)
        {
            IPacketBuffer* Buffer = CreateIBufferInstanceEx((BYTE*)Packet->Data, Packet->Length - BASE_PACKET_HEADER_LEN);
            free(Packet);
            return Buffer;
        }

        //if cache not has a packet, read 4k byte to cache
        BYTE data[4096];
        DWORD read = 0;
        BOOL Ret = SocketRead(m_socket, data, 4096, &read, StopEvent);

        //read fail, stop recv
        if (Ret == FALSE)
        {
            return NULL;
        }

        m_Cache->AddData(data, read);
    } while (TRUE);

    return NULL;
}

BOOL CSocketBase::SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent)
{
    if (!Buffer->DataPush(BASE_PACKET_HEADER_LEN))
    {
        return FALSE;
    }

    BASE_PACKET_T* Packet = (BASE_PACKET_T*)Buffer->GetData();

    Packet->Length = Buffer->GetBufferLength();
    Packet->Type = 0;

    DWORD Len = Packet->Length;
    PBYTE Offset = Buffer->GetData();

    while (Len > 0)
    {
        DWORD Write = 0;
        if (SocketWrite(m_socket, Offset, Len, &Write, StopEvent))
        {
            Offset += Write;
            Len -= Write;
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

void CSocketBase::SocketClear(ICommunication* param)
{
    CSocketBase *Socket = dynamic_cast<CSocketBase *>(param);

    if (Socket->m_bAlive)
    {
        Socket->StopCommunication();

        if (Socket->m_socket != INVALID_SOCKET)
        {
            shutdown(Socket->m_socket, SD_BOTH);
            closesocket(Socket->m_socket);
            Socket->m_socket = INVALID_SOCKET;
        }

        Socket->m_bAlive = FALSE;
    }
}

VOID CSocketBase::GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort)
{
    strncpy(SrcAddress, m_szSrcAddress, BufferLen);

    *SrcPort = m_dwSrcPort;

    return;
}

VOID CSocketBase::GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort)
{
    strncpy(DstAddress, m_szDstAddress, BufferLen);

    *DstPort = m_dwDstPort;

    return;
}