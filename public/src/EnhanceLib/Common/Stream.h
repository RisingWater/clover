#pragma once

#ifndef __PKT_STREAM_H__
#define __PKT_STREAM_H__

#include "Common/IStream.h"
#include "Base/BaseObject.h"
#include "Common/IBuffer.h"

class CPacketReadStream : public virtual IPacketReadStream
{
public:
    CPacketReadStream(IPacketBuffer* Buffer);

    ~CPacketReadStream();

    virtual BYTE WINAPI ReadINT8();

    virtual SHORT WINAPI ReadINT16();

    virtual DWORD WINAPI ReadINT32();

    virtual ULONGLONG WINAPI ReadINT64();

    virtual DWORD WINAPI DataLeft();

    virtual VOID WINAPI ReadBuffer(PVOID Value, DWORD Len);

protected:
    IPacketBuffer* m_pReadBuffer;
    PBYTE          m_pData;
    DWORD          m_dwDataLeft;
};

class CPacketWriteStream : public virtual IPacketWriteStream
{
public:
    CPacketWriteStream(DWORD len);

    ~CPacketWriteStream();

    virtual VOID WINAPI WriteINT8(BYTE Value);

    virtual VOID WINAPI WriteINT16(SHORT Value);

    virtual VOID WINAPI WriteINT32(DWORD Value);

    virtual VOID WINAPI WriteINT64(ULONGLONG Value);

    virtual VOID WINAPI WriteBuffer(PVOID Value, DWORD Len);

    virtual DWORD WINAPI DataSize();

    virtual IPacketBuffer* GetPacketBuffer();

protected:
    PBYTE m_pBuffer;
    PBYTE m_pData;
    DWORD m_dwBufferLen;
    DWORD m_dwDataLen;
};

#endif
