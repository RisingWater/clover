#include "stdafx.h"
#include "CloverChannel.h"
#include "CloverLink.h"

CCloverChannel::CCloverChannel(CCloverLink *xlink, CLOVER_LINK_CHANNEL_ITEM *channelParam)
{
    m_pLink = xlink;
    if (m_pLink)
    {
        m_pLink->AddRef();
    }

    memcpy(&m_stChannelParam, channelParam, sizeof(CLOVER_LINK_CHANNEL_ITEM));

    m_pfnRecvCallback = NULL;
    m_pRecvCallbackCtx = NULL;

    m_bClosed = FALSE;

    m_hUnProcessDataEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection(&m_csRecvLock);
}

CCloverChannel::~CCloverChannel()
{
    if (m_pLink)
    {
        m_pLink->Release();
        m_pLink = NULL;
    }

    if (m_hStopEvent)
    {
        CloseHandle(m_hStopEvent);
    }

    if (m_hUnProcessDataEvent)
    {
        CloseHandle(m_hUnProcessDataEvent);
    }

    DeleteCriticalSection(&m_csRecvLock);
}

const char* CCloverChannel::GetChannelName()
{
    return m_stChannelParam.chanName;
}

int CCloverChannel::GetPrioriy()
{
    return m_stChannelParam.priority;
}

int CCloverChannel::GetChannelID()
{
    return m_stChannelParam.channelID;
}

void CCloverChannel::Close()
{
    EnterCriticalSection(&m_csRecvLock);
    m_bClosed = TRUE;

    if (m_pfnRecvCallback)
    {
        m_pfnRecvCallback(this, NET_CHANNEL_CLOSED, NULL, m_pRecvCallbackCtx);
        m_pfnRecvCallback = NULL;
        m_pRecvCallbackCtx = NULL;
    }

    LeaveCriticalSection(&m_csRecvLock);

    return;
}

BOOL CCloverChannel::Send(IPacketBuffer* Buffer)
{
    EnterCriticalSection(&m_csRecvLock);
    if (m_bClosed)
    {
		LeaveCriticalSection(&m_csRecvLock);
        return FALSE;
    }

    LeaveCriticalSection(&m_csRecvLock);

    return m_pLink->ChannelSend(Buffer, this);
}

void CCloverChannel::SetRecvCallback(_CloverChannelRecvCallback recvCallBack, void *context)
{
    EnterCriticalSection(&m_csRecvLock);
    m_pRecvCallbackCtx = context;
    m_pfnRecvCallback = recvCallBack;

    if (m_pfnRecvCallback)
    {
        std::list<IPacketBuffer*>::iterator Itor;

        for (Itor = m_unProcessDataList.begin(); Itor != m_unProcessDataList.end(); Itor++)
        {
            IPacketBuffer* channelBuf = (*Itor);
            m_pfnRecvCallback(this, NET_SUCCESS, channelBuf, m_pRecvCallbackCtx);
            channelBuf->Release();
        }

        m_unProcessDataList.clear();
    }
    LeaveCriticalSection(&m_csRecvLock);

    return;
}

BOOL CCloverChannel::Recv(IPacketBuffer** Buffer)
{
    HANDLE h[2] = {
        m_hUnProcessDataEvent,
        m_hStopEvent
    };

    DWORD Ret = WaitForMultipleObjects(2, h, FALSE, INFINITE);

    if (Ret == WAIT_OBJECT_0)
    {
        EnterCriticalSection(&m_csRecvLock);
        if (!m_unProcessDataList.empty())
        {
            IPacketBuffer* Packet = m_unProcessDataList.front();
            m_unProcessDataList.pop_front();
            *Buffer = Packet;
        }
        else
        {
            LeaveCriticalSection(&m_csRecvLock);
            return FALSE;
        }

        if (m_unProcessDataList.empty())
        {
            ResetEvent(m_hUnProcessDataEvent);
        }

        LeaveCriticalSection(&m_csRecvLock);

        return TRUE;
    }
    else
    {
        *Buffer = NULL;
        return FALSE;
    }
}

BOOL CCloverChannel::Recv(IPacketBuffer* Buffer)
{
    EnterCriticalSection(&m_csRecvLock);
    if (m_bClosed)
    {
		LeaveCriticalSection(&m_csRecvLock);
        return TRUE;
    }

    if (m_pfnRecvCallback)
    {
        m_pfnRecvCallback(this, NET_SUCCESS, Buffer, m_pRecvCallbackCtx);
    }
    else
    {
        Buffer->AddRef();
        m_unProcessDataList.push_back(Buffer);
        SetEvent(m_hUnProcessDataEvent);
    }

    LeaveCriticalSection(&m_csRecvLock);

    return TRUE;
}