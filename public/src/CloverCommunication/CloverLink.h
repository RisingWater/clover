#ifndef __X_LINK_H__
#define __X_LINK_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "CloverCommunication.h"
#include "Base/BaseObject.h"
#include <list>

class CTCPBase;
class CCloverChannel;

class CCloverLink : public ICloverLink
{
public:
    CCloverLink(CHAR* ServerIP, WORD ServerTCPPort);
	CCloverLink(SOCKET socket, WORD ServerTCPPort);

    virtual ~CCloverLink();

    BOOL Connect();

    virtual void CLOVER_API ChannelsReady(_CloverRecvFailCallback callback, PVOID param);

    virtual void CLOVER_API Close();

    virtual BOOL CLOVER_API Send(IPacketBuffer* Buffer);

    virtual BOOL CLOVER_API Recv(IPacketBuffer* Buffer);

    virtual void CLOVER_API SetSendTimeout(DWORD timeout);

    virtual void CLOVER_API GetSendTimeout(DWORD *timeout);

    virtual void CLOVER_API SetRecvTimeout(DWORD timeout);

    virtual void CLOVER_API GetRecvTimeout(DWORD *timeout);

    virtual BOOL CLOVER_API SetChannels(int numOfChannel, CLOVER_LINK_CHANNEL_ITEM *channelList);

    virtual BOOL CLOVER_API CreateChannel(const char *channelName, ICloverChannel** channelParam);

    virtual void CLOVER_API SetSecurityKey(SecurityKey* SecurityData);

    virtual void CLOVER_API GetPublicKey(SecurityKey* SecurityData);

    BOOL ChannelSend(IPacketBuffer* Buffer, CCloverChannel* channel);

private:
	void InitParam();

    static BOOL RecvProcessDelegate(IPacketBuffer* Buffer, CTCPBase* tcp, CBaseObject* Param);

    static VOID ConnectEndProcessDelegate(CTCPBase* tcp, CBaseObject* Param);

    BOOL RecvProcess(IPacketBuffer* Buffer, CTCPBase* tcp);

    VOID ConnectEndProcess(CTCPBase* tcp);

	CTCPBase* m_pTCPBase;
    HANDLE m_hStopEvent;

    DWORD m_dwSendTimeout;
    DWORD m_dwRecvTimeout;

    _CloverRecvFailCallback m_pfnFailFunc;
    PVOID m_pFailParam;
    CRITICAL_SECTION m_csFailFuncLock;

    CCloverChannel *m_ChannelList[CLOVER_NUMBER_OF_CHANNEL];
    CRITICAL_SECTION m_csChannelListLock;

    BOOL m_bClosed;
    CRITICAL_SECTION m_csCloseLock;

    CRITICAL_SECTION m_csBasePacketList;
    HANDLE m_hBaseChannelRecvEvent;
    HANDLE m_hDataBuffStream;

    SecurityKey m_szOurSecretKey;
    SecurityKey m_szOurPublicKey;
    SecurityKey m_szTheirPublicKey;
};

#endif
