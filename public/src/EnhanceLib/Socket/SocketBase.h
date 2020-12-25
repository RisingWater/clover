#ifndef __SOCKET_BASE_H__
#define __SOCKET_BASE_H__

#include <string>
#include "Common/Communication.h"
#include "Common/Cache.h"
#include "Socket/ISocketBase.h"

class CSocketBase : public virtual CCommunication, public virtual ISocketBase
{
public:
    CSocketBase(SocketType Type);

    ~CSocketBase();

    virtual void WINAPI Close();

    virtual PVOID WINAPI GetParam(const CHAR* ParamKeyword);

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, PVOID Param);

    virtual VOID GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort);

    virtual VOID GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort);

protected:
    BOOL WINAPI Open();

    SOCKET m_socket;
    SocketType m_eSocketType;

    CHAR   m_szDstAddress[128];
    WORD  m_dwDstPort;

    CHAR   m_szSrcAddress[128];
    WORD  m_dwSrcPort;

private:
    virtual IPacketBuffer* RecvAPacket(HANDLE StopEvent);

    virtual BOOL SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent);

    static void SocketClear(ICommunication* param);

    BOOL   m_bAlive;
    std::map<std::string, PVOID> m_ParamMap;
    CRITICAL_SECTION             m_csParamLock;
    CCache* m_Cache;
};

#endif