#ifndef __CLOVER_COMMUNICATION_H__
#define __CLOVER_COMMUNICATION_H__

#ifdef _WIN32
#define CLOVER_API __stdcall
#include <windows.h>
#else
#define XAPI
#include <winpr/wtypes.h>
#include <winpr/tchar.h>
#include <winpr/synch.h>
#endif

#include "Base/BaseObject.h"

#define CLOVER_NUMBER_OF_CHANNEL 255
#define CLOVER_NUM_OF_CHANNEL_PRIORITY 8

#define NET_SUCCESS        0x0
#define NET_LINK_CLOSE     0x1000
#define NET_CHANNEL_CLOSED 0x1001

typedef struct
{
    CHAR chanName[32];
    BYTE channelID;
    BYTE priority;
} CLOVER_LINK_CHANNEL_ITEM;

#define SECURITY_KEYSIZE 32

typedef struct
{
    UCHAR SecretKey[SECURITY_KEYSIZE];
} SecurityKey;

class ICloverComm;
class ICloverLink;
class ICloverChannel;
class IPacketBuffer;

typedef void (CLOVER_API *_CloverAcceptProcess)(ICloverLink* Link, CBaseObject* pParam);
typedef void (CLOVER_API *_CloverRecvFailCallback)(int ret, PVOID callback);
typedef void (CLOVER_API *_CloverChannelRecvCallback)(ICloverChannel* channel, int retCode, IPacketBuffer* buffer, void *context);

class ICloverComm : public CBaseObject
{
public:
    virtual BOOL CLOVER_API Listen(WORD Port) = 0;
	virtual void CLOVER_API RegisterAcceptCallback(_CloverAcceptProcess Process, CBaseObject* param) = 0;
	virtual BOOL CLOVER_API Connect(CHAR* IPAddress, WORD Port, ICloverLink ** xlk) = 0;
	virtual void CLOVER_API Close() = 0;
};

class ICloverLink : public CBaseObject
{
public:
    virtual void CLOVER_API Close() = 0;

    virtual BOOL CLOVER_API Send(IPacketBuffer* Buffer) = 0;

    virtual void CLOVER_API SetSendTimeout(DWORD timeout) = 0;

    virtual void CLOVER_API GetSendTimeout(DWORD* timeout) = 0;

    virtual void CLOVER_API SetRecvTimeout(DWORD timeout) = 0;

    virtual void CLOVER_API GetRecvTimeout(DWORD* timeout) = 0;

    virtual BOOL CLOVER_API Recv(IPacketBuffer* Buffer) = 0;

    virtual BOOL CLOVER_API SetChannels(int numOfChannel, CLOVER_LINK_CHANNEL_ITEM *channelList) = 0;

    virtual void CLOVER_API ChannelsReady(_CloverRecvFailCallback callback = NULL, PVOID param = NULL) = 0;

    virtual BOOL CLOVER_API CreateChannel(const char *channelName, ICloverChannel** channel) = 0;

    virtual void CLOVER_API SetSecurityKey(SecurityKey* SecurityData) = 0;

    virtual void CLOVER_API GetPublicKey(SecurityKey* SecurityData) = 0;
};

class ICloverChannel : public CBaseObject
{
public:
    virtual void CLOVER_API Close() = 0;

    virtual void CLOVER_API SetRecvCallback(_CloverChannelRecvCallback recvCallBack, void *context) = 0;

    virtual BOOL CLOVER_API Send(IPacketBuffer* Buffer) = 0;

    virtual BOOL CLOVER_API Recv(IPacketBuffer** Buffer) = 0;
};

extern "C"
{
    ICloverComm* CLOVER_API CreateCloverComm();
}

#endif
