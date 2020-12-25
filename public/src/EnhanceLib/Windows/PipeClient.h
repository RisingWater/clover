#pragma once

#ifndef __PIPE_CLIENT_H__
#define __PIPE_CLIENT_H__

#include "Windows/IPipeClient.h"
#include "Common/Communication.h"

class CPipeClient : public IPipeClient, public CCommunication
{
public:
    CPipeClient(TCHAR *PipeName, DWORD Timeout);

    virtual ~CPipeClient();

    virtual BOOL WINAPI Connect();

    virtual void WINAPI DisConnect();

    virtual BOOL WINAPI IsConnected();

    virtual CBaseObject* WINAPI GetParam(const CHAR* ParamKeyword);

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CBaseObject* Param);

private:
    virtual IPacketBuffer* RecvAPacket(HANDLE StopEvent);

    virtual BOOL SendAPacket(IPacketBuffer* Buffer, HANDLE StopEvent);

    static void PipeClear(ICommunication* param);

    HANDLE InitSyncLock();

    HANDLE                       m_hPipe;
    TCHAR*                       m_szPipeName;
    DWORD                        m_dwTimeout;
    std::map<UINT32, CBaseObject*> m_ParamMap;
    CRITICAL_SECTION             m_csParamLock;
};

#endif