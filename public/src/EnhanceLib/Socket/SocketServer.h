#ifndef __SOCKET_SERVER_H__
#define __SOCKET_SERVER_H__

#include <string>
#include "Socket/ISocketServer.h"
#include "Socket/SocketBase.h"

class CSocketServer : public CSocketBase, public ISocketServer
{
public:
    CSocketServer(SOCKET socket, const CHAR* DstAddress, WORD DstPort);

    CSocketServer(SOCKET socket);

    ~CSocketServer();

    virtual BOOL WINAPI Start();

private:
    //release self when disconnected
    static void SelfRelease(ICommunication* param);
};

class CSocketServerService : public ISocketServerService
{
public:
    CSocketServerService(WORD Port, HANDLE StopEvent);

#ifndef WIN32
    CSocketServerService(CHAR* FileName, HANDLE StopEvent);
#endif

    ~CSocketServerService();

    virtual BOOL WINAPI StartMainService();

    virtual void WINAPI StopMainService();

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestPacketHandle Func);

    virtual BOOL WINAPI RegisterRequestHandle(DWORD Type, RequestDataHandle Func);

    virtual void WINAPI RegisterEndHandle(EndHandle Func);

    virtual void WINAPI RegisterConnectHandle(ConnectHandle Func);

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, PVOID Param);

private:
    static BOOL ServiceMainThreadProc(LPVOID Parameter, HANDLE StopEvent);

    void InitalizeServer(CSocketServer* Server);

    IThread*                             m_pMainThread;
    CHAR                                 m_szSrcAddress[128];
    WORD                                 m_dwSrcPort;
    CHAR*                                m_szFileName;
    SocketType                           m_eSocketType;
    HANDLE                               m_hStopEvent;
    CRITICAL_SECTION                     m_csLock;
    std::map<DWORD, RequestPacketHandle> m_ReqPacketList;
    std::map<DWORD, RequestDataHandle>   m_ReqDataList;
    std::list<EndHandle>                 m_EndList;
    std::list<ConnectHandle>             m_ConnectList;
    std::map<std::string, PVOID>         m_ParamMap;
    SOCKET                               m_ListenSocket;
};

#endif