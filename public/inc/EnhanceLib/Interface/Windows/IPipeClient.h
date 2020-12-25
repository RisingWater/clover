﻿#pragma once

#ifndef __IPIPE_CLIENT_H__
#define __IPIPE_CLIENT_H__

#include "Common/ICommunication.h"
#include "DllExport.h"

class DLL_COMMONLIB_API IPipeClient : public virtual ICommunication
{
public:
    virtual BOOL WINAPI Connect() = 0;

    virtual void WINAPI DisConnect() = 0;

    virtual BOOL WINAPI IsConnected() = 0;

    virtual CBaseObject* WINAPI GetParam(const CHAR* ParamKeyword) = 0;

    virtual VOID WINAPI SetParam(const CHAR* ParamKeyword, CBaseObject* Param) = 0;
};

extern "C"
{
    DLL_COMMONLIB_API IPipeClient* WINAPI CreateIPipeClientInstance(TCHAR *PipeName, DWORD Timeout);
}

#endif