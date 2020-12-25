#ifndef __XCHANNEL_H__
#define __XCHANNEL_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "CloverCommunication.h"
#include "Base/BaseObject.h"
#include "Common/BasePacket.h"
#include "Common/IBuffer.h"
#include <list>

class CCloverLink;

typedef struct _XRECV_CHANNEL_BUF_T
{
    BYTE* buf;
    DWORD length;
} XRECV_CHANNEL_BUF_T;

class CCloverChannel : public ICloverChannel
{
public:
    CCloverChannel(CCloverLink *xlink, CLOVER_LINK_CHANNEL_ITEM *channelParam);
    virtual ~CCloverChannel();

    virtual void CLOVER_API Close();
    virtual void CLOVER_API SetRecvCallback(_CloverChannelRecvCallback recvCallBack, void *context);
    virtual BOOL CLOVER_API Send(IPacketBuffer* Buffer);
    virtual BOOL CLOVER_API Recv(IPacketBuffer** Buffer);

    BOOL Recv(IPacketBuffer* Buffer);

    const char *GetChannelName();
    int GetPrioriy();
    int GetChannelID();

private:
    CCloverLink *m_pLink;
    CLOVER_LINK_CHANNEL_ITEM m_stChannelParam;

    CRITICAL_SECTION m_csRecvLock;
    _CloverChannelRecvCallback m_pfnRecvCallback;
    void* m_pRecvCallbackCtx;
    BOOL m_bClosed;

    std::list<IPacketBuffer*> m_unProcessDataList;
    HANDLE m_hUnProcessDataEvent;
    HANDLE m_hStopEvent;
};

#endif
