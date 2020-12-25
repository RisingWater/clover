#include "stdafx.h"
#include "CloverLink.h"
#include "TCPClient.h"
#include "TCPServer.h"
#include "CloverChannel.h"
#include "Common/IDataStreamBuf.h"
#include <sodium.h>

CCloverLink::CCloverLink(SOCKET socket, WORD ServerTCPPort) : ICloverLink()
{
	m_pTCPBase = new CTCPServer(socket, ServerTCPPort);

	InitParam();

	EnterCriticalSection(&m_csCloseLock);
	m_bClosed = FALSE;
	LeaveCriticalSection(&m_csCloseLock);
}

CCloverLink::CCloverLink(CHAR* ServerIP, WORD ServerTCPPort) : ICloverLink()
{
	m_pTCPBase = new CTCPClient(ServerIP, ServerTCPPort);

	InitParam();
}

void CCloverLink::InitParam()
{
    m_dwSendTimeout = INFINITE;
    m_dwRecvTimeout = INFINITE;

    m_bClosed = TRUE;

    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hBaseChannelRecvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_pfnFailFunc = NULL;
    m_pFailParam = NULL;

    m_hDataBuffStream = CreateDataStreamBuffer();

    memset(&m_ChannelList, 0, sizeof(CCloverChannel*) * CLOVER_NUMBER_OF_CHANNEL);

    memset(&m_szOurSecretKey, 0, SECURITY_KEYSIZE);
    memset(&m_szOurPublicKey, 0, SECURITY_KEYSIZE);
    memset(&m_szTheirPublicKey, 0, SECURITY_KEYSIZE);

    InitializeCriticalSection(&m_csCloseLock);
    InitializeCriticalSection(&m_csFailFuncLock);
    InitializeCriticalSection(&m_csBasePacketList);
    InitializeCriticalSection(&m_csChannelListLock);

	crypto_box_keypair(m_szOurPublicKey.SecretKey, m_szOurSecretKey.SecretKey);

	m_pTCPBase->RegisterRecvProcess(CCloverLink::RecvProcessDelegate, this);
	m_pTCPBase->RegisterEndProcess(CCloverLink::ConnectEndProcessDelegate, this);
}

CCloverLink::~CCloverLink()
{
    if (m_pTCPBase)
    {
        m_pTCPBase->Release();
        m_pTCPBase = NULL;
    }

    if (m_hBaseChannelRecvEvent)
    {
        CloseHandle(m_hBaseChannelRecvEvent);
        m_hBaseChannelRecvEvent = NULL;
    }

    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
        m_hStopEvent = NULL;
    }

    if (m_hDataBuffStream)
    {
        CloseDataStreamBuffer(m_hDataBuffStream);
    }

    DeleteCriticalSection(&m_csCloseLock);
    DeleteCriticalSection(&m_csFailFuncLock);
    DeleteCriticalSection(&m_csBasePacketList);
    DeleteCriticalSection(&m_csChannelListLock);
}

BOOL CCloverLink::Connect()
{
    CTCPClient* host = dynamic_cast<CTCPClient*>(m_pTCPBase);

    BOOL Ret = host->Connect();

    EnterCriticalSection(&m_csCloseLock);
    m_bClosed = FALSE;
    LeaveCriticalSection(&m_csCloseLock);

    return Ret;
}

void CCloverLink::Close()
{
	m_pTCPBase->RegisterRecvProcess(NULL, NULL);
	m_pTCPBase->RegisterEndProcess(NULL, NULL);

    EnterCriticalSection(&m_csCloseLock);

    if (m_bClosed)
    {
        LeaveCriticalSection(&m_csCloseLock);
        return;
    }

    m_bClosed = TRUE;

    LeaveCriticalSection(&m_csCloseLock);

    EnterCriticalSection(&m_csChannelListLock);

    for (int i = 0; i < CLOVER_NUMBER_OF_CHANNEL; i++)
    {
        if (m_ChannelList[i])
        {
            m_ChannelList[i]->Close();
            m_ChannelList[i]->Release();
            m_ChannelList[i] = NULL;
        }
    }

    LeaveCriticalSection(&m_csChannelListLock);

    SetEvent(m_hStopEvent);

    EnterCriticalSection(&m_csFailFuncLock);

    if (m_pfnFailFunc)
    {
        m_pfnFailFunc(NET_LINK_CLOSE, m_pFailParam);

        m_pfnFailFunc = NULL;
        m_pFailParam = NULL;
    }

    LeaveCriticalSection(&m_csFailFuncLock);

    if (m_pTCPBase)
    {
        m_pTCPBase->Close();
    }

    return;
}

void CCloverLink::SetSendTimeout(DWORD timeout)
{
    m_dwSendTimeout = timeout;
    return;
}

void CCloverLink::GetSendTimeout(DWORD *timeout)
{
    if (timeout)
    {
        *timeout = m_dwSendTimeout;
        return;
    }
    else
    {
        return;
    }
}

void CCloverLink::SetRecvTimeout(DWORD timeout)
{
    m_dwRecvTimeout = timeout;
    return;
}

void CCloverLink::GetRecvTimeout(DWORD *timeout)
{
    if (timeout)
    {
        *timeout = m_dwRecvTimeout;
        return;
    }
    else
    {
        return;
    }
}

void CCloverLink::ChannelsReady(_CloverRecvFailCallback callback, PVOID param)
{
    EnterCriticalSection(&m_csFailFuncLock);
    m_pfnFailFunc = callback;
    m_pFailParam = param;
    LeaveCriticalSection(&m_csFailFuncLock);

    return;
}

VOID CCloverLink::ConnectEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param)
{
    CCloverLink* link = dynamic_cast<CCloverLink*>(Param);

    if (link)
    {
        link->ConnectEndProcess(tcp);
    }

    return;
}

BOOL CCloverLink::RecvProcessDelegate(IPacketBuffer* Buffer, CTCPBase* tcp, CBaseObject* Param)
{
    CCloverLink* link = dynamic_cast<CCloverLink*>(Param);

    if (link)
    {
        link->RecvProcess(Buffer, tcp);
    }

    return TRUE;
}

VOID CCloverLink::ConnectEndProcess(CTCPBase* tcp)
{
    Close();
}

BOOL CCloverLink::RecvProcess(IPacketBuffer* Buffer, CTCPBase* tcp)
{
    BASE_PACKET_T* Packet = (BASE_PACKET_T*)Buffer->GetData();
    Buffer->DataPull(BASE_PACKET_HEADER_LEN);

    if (Packet->Type == CLOVER_NUMBER_OF_CHANNEL)
    {
        EnterCriticalSection(&m_csBasePacketList);
        DataStreamBufferAddData(m_hDataBuffStream, Buffer->GetData(), Buffer->GetBufferLength());
        SetEvent(m_hBaseChannelRecvEvent);
        LeaveCriticalSection(&m_csBasePacketList);
    }
    else
    {
        CCloverChannel* Channel = NULL;
        EnterCriticalSection(&m_csChannelListLock);
        Channel = m_ChannelList[Packet->Type];
        if (Channel)
        {
            Channel->AddRef();
        }
        LeaveCriticalSection(&m_csChannelListLock);

        if (Channel)
        {
            IPacketBuffer* DecryptedPkt = CreateIBufferInstance(Buffer->GetBufferLength() - crypto_box_SEALBYTES);

            int ret = crypto_box_seal_open(DecryptedPkt->GetData(), Buffer->GetData(), Buffer->GetBufferLength(), m_szOurPublicKey.SecretKey, m_szOurSecretKey.SecretKey);

            if (ret != 0)
            {
                L_TRACE(_T("crypto_box_seal_open %d\r\n"), ret);
            }
            else
            {
                Channel->Recv(DecryptedPkt);
            }

            DecryptedPkt->Release();
            Channel->Release();
        }
    }

    return TRUE;
}

BOOL CCloverLink::Recv(IPacketBuffer* Buffer)
{
    HANDLE h[2] = {
        m_hBaseChannelRecvEvent,
        m_hStopEvent
    };

    INT TimeoutRest = m_dwRecvTimeout;

    while (TRUE)
    {
        DWORD Start = GetTickCount();
        DWORD Ret = WaitForMultipleObjects(2, h, FALSE, TimeoutRest);

        DWORD UseTime = GetTickCount() - Start;

        if (Ret == WAIT_OBJECT_0)
        {
            EnterCriticalSection(&m_csBasePacketList);

            DWORD Size = DataStreamBufferGetCurrentDataSize(m_hDataBuffStream);

            if (Size >= Buffer->GetBufferLength())
            {
                DWORD readed;
                DataStreamBufferGetData(m_hDataBuffStream, Buffer->GetData(), Buffer->GetBufferLength(), &readed);

                LeaveCriticalSection(&m_csBasePacketList);
                return TRUE;
            }
            else
            {
                ResetEvent(m_hBaseChannelRecvEvent);
            }

            LeaveCriticalSection(&m_csBasePacketList);

            TimeoutRest -= UseTime;

            if (TimeoutRest < 0)
            {
                TimeoutRest = 0;
            }
        }
        else
        {
            return FALSE;
        }
    }
}

BOOL CCloverLink::Send(IPacketBuffer* Buffer)
{
    DWORD DataLen = Buffer->GetBufferLength();

    EnterCriticalSection(&m_csCloseLock);
    if (m_bClosed)
    {
        LeaveCriticalSection(&m_csCloseLock);
        return FALSE;
    }
    LeaveCriticalSection(&m_csCloseLock);

    Buffer->DataPush(BASE_PACKET_HEADER_LEN);

    BASE_PACKET_T* Packet = (BASE_PACKET_T*)Buffer->GetData();
    Packet->Type = CLOVER_NUMBER_OF_CHANNEL;
    Packet->Length = Buffer->GetBufferLength();

    m_pTCPBase->SendPacket(Buffer);

    return TRUE;
}

BOOL CCloverLink::ChannelSend(IPacketBuffer* Buffer, CCloverChannel* channel)
{
    EnterCriticalSection(&m_csCloseLock);
    if (m_bClosed)
    {
        LeaveCriticalSection(&m_csCloseLock);
        return FALSE;
    }
    LeaveCriticalSection(&m_csCloseLock);

    IPacketBuffer* EncryptedPkt = CreateIBufferInstance(Buffer->GetBufferLength() + crypto_box_SEALBYTES);

    int ret = crypto_box_seal(EncryptedPkt->GetData(), Buffer->GetData(), Buffer->GetBufferLength(), m_szTheirPublicKey.SecretKey);

    if (ret != 0)
    {
        L_ERROR(_T("crypto_box_seal %d\r\n"), ret);
    }
    else
    {
        EncryptedPkt->DataPush(BASE_PACKET_HEADER_LEN);
        BASE_PACKET_T* Packet = (BASE_PACKET_T*)EncryptedPkt->GetData();

        Packet->Type = channel->GetChannelID();
        Packet->Length = EncryptedPkt->GetBufferLength();

        m_pTCPBase->SendPacket(EncryptedPkt);

        EncryptedPkt->Release();
    }

    return TRUE;
}

BOOL CCloverLink::SetChannels(int numOfChannel, CLOVER_LINK_CHANNEL_ITEM *channelList)
{
    int i;
    if (numOfChannel == 0
        || numOfChannel >= CLOVER_NUMBER_OF_CHANNEL
        || channelList == NULL)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csChannelListLock);
    for (i = 0; i < numOfChannel; i++)
    {
        int channelID = channelList[i].channelID;
        if (channelID >= CLOVER_NUMBER_OF_CHANNEL)
        {
            L_ERROR(_T("invalid channelID %d\n"), channelList[i].channelID);
            return FALSE;
        }
        m_ChannelList[channelID] = new CCloverChannel(this, &(channelList[i]));
    }
    LeaveCriticalSection(&m_csChannelListLock);

    return TRUE;
}

BOOL CCloverLink::CreateChannel(const char *channelName, ICloverChannel** channelParam)
{
    int i;
    ICloverChannel *channel = NULL;

    if (channelName == NULL || channelParam == NULL)
    {
        return FALSE;
    }

    EnterCriticalSection(&m_csChannelListLock);

    if (m_bClosed)
    {
        LeaveCriticalSection(&m_csChannelListLock);
        return FALSE;
    }

    for (i = 0; i < CLOVER_NUMBER_OF_CHANNEL; i++)
    {
        CCloverChannel *channelTmp = m_ChannelList[i];

        if (channelTmp)
        {
            const char *channelNameTmp = channelTmp->GetChannelName();
            if (strncmp(channelNameTmp, channelName, 8) == 0)
            {
                channel = channelTmp;
                channel->AddRef();
                break;
            }
        }
    }
    LeaveCriticalSection(&m_csChannelListLock);

    if (!channel)
    {
        return FALSE;
    }
    else
    {
        *channelParam = channel;
        return TRUE;
    }
}

void CCloverLink::SetSecurityKey(SecurityKey* SecurityData)
{
    memcpy(&m_szTheirPublicKey, SecurityData, SECURITY_KEYSIZE);

    L_TRACE(_T("SetSecurityKey\r\n"));
    L_TMPDUMP(m_szTheirPublicKey.SecretKey, SECURITY_KEYSIZE);

    return;
}

void CCloverLink::GetPublicKey(SecurityKey* SecurityData)
{
    memcpy(SecurityData, &m_szOurPublicKey, SECURITY_KEYSIZE);

    L_TRACE(_T("GetPublicKey\r\n"));
    L_TMPDUMP(m_szOurPublicKey.SecretKey, SECURITY_KEYSIZE);

    return;
}