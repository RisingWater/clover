#pragma once

#ifndef __ISOCKET_BASE_H__
#define __ISOCKET_BASE_H__

#include "Common/ICommunication.h"
#include "DllExport.h"

typedef enum
{
    FILE_SOCKET = 0,
    NET_SOCKET,
} SocketType;

class DLL_COMMONLIB_API ISocketBase : public virtual ICommunication
{
public:
    virtual void WINAPI Close() = 0;

    virtual PVOID WINAPI GetParam(const CHAR* ParamKeyword) = 0;

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, PVOID Param) = 0;

    virtual VOID GetSrcPeer(CHAR* SrcAddress, DWORD BufferLen, WORD* SrcPort) = 0;

    virtual VOID GetDstPeer(CHAR* DstAddress, DWORD BufferLen, WORD* DstPort) = 0;
};

#endif