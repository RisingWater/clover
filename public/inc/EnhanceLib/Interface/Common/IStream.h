#pragma once

#ifndef __PKT_ISTREAM_H__
#define __PKT_ISTREAM_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "DllExport.h"
#include "Common/IBuffer.h"

class DLL_COMMONLIB_API IPacketReadStream : public virtual CBaseObject
{
public:
    virtual BYTE WINAPI ReadINT8() = 0;

    virtual SHORT WINAPI ReadINT16() = 0;

    virtual DWORD WINAPI ReadINT32() = 0;

    virtual ULONGLONG WINAPI ReadINT64() = 0;

    virtual VOID WINAPI ReadBuffer(PVOID Value, DWORD Len) = 0;

    virtual DWORD WINAPI DataLeft() = 0;
};

class DLL_COMMONLIB_API IPacketWriteStream : public virtual CBaseObject
{
public:
    virtual VOID WINAPI WriteINT8(BYTE Value) = 0;

    virtual VOID WINAPI WriteINT16(SHORT Value) = 0;

    virtual VOID WINAPI WriteINT32(DWORD Value) = 0;

    virtual VOID WINAPI WriteINT64(ULONGLONG Value) = 0;

    virtual VOID WINAPI WriteBuffer(PVOID Value, DWORD Len) = 0;

    virtual DWORD WINAPI DataSize() = 0;

    virtual IPacketBuffer* GetPacketBuffer() = 0;
};

extern "C"
{
    DLL_COMMONLIB_API IPacketReadStream* WINAPI CreateIReadStreamInstance(IPacketBuffer* Buffer);

    DLL_COMMONLIB_API IPacketWriteStream* WINAPI CreateIWriteStreamInstance(DWORD Len);
}

#endif