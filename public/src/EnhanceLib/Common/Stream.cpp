#include "StdAfx.h"
#include "Stream.h"

CPacketReadStream::CPacketReadStream(IPacketBuffer* Buffer) : CBaseObject()
{
    m_pReadBuffer = Buffer;
    m_pReadBuffer->AddRef();

    m_pData = m_pReadBuffer->GetData();
    m_dwDataLeft = m_pReadBuffer->GetBufferLength();
}

CPacketReadStream::~CPacketReadStream()
{
    m_pReadBuffer->Release();
}

BYTE CPacketReadStream::ReadINT8()
{
    BYTE Value = *m_pData;

    m_pData += 1;
    m_dwDataLeft -= 1;

    return Value;
}

SHORT CPacketReadStream::ReadINT16()
{
    SHORT Value = *(SHORT*)m_pData;

    m_pData += 2;
    m_dwDataLeft -= 2;

    return Value;
}

DWORD CPacketReadStream::ReadINT32()
{
    DWORD Value = *(DWORD*)m_pData;

    m_pData += 4;
    m_dwDataLeft -= 4;

    return Value;
}

ULONGLONG CPacketReadStream::ReadINT64()
{
    ULONGLONG Value = *(ULONGLONG*)m_pData;

    m_pData += 8;
    m_dwDataLeft -= 8;

    return Value;
}

VOID CPacketReadStream::ReadBuffer(PVOID Value, DWORD Len)
{
    memcpy(Value, m_pData, Len);

    m_pData += Len;
    m_dwDataLeft -= Len;
}

DWORD CPacketReadStream::DataLeft()
{
    return m_dwDataLeft;
}

CPacketWriteStream::CPacketWriteStream(DWORD len)
{
    m_pBuffer = (PBYTE)malloc(len);
    m_dwBufferLen = len;

    m_pData = m_pBuffer;
    m_dwDataLen = 0;
}

CPacketWriteStream::~CPacketWriteStream()
{
    if (m_pBuffer)
    {
        free(m_pBuffer);
    }
}

VOID CPacketWriteStream::WriteINT8(BYTE Value)
{
    *m_pData = Value;

    m_pData += 1;
    m_dwDataLen += 1;
}

VOID CPacketWriteStream::WriteINT16(SHORT Value)
{
    *(SHORT*)m_pData = Value;

    m_pData += 2;
    m_dwDataLen += 2;
}

VOID CPacketWriteStream::WriteINT32(DWORD Value)
{
    *(DWORD*)m_pData = Value;

    m_pData += 4;
    m_dwDataLen += 4;
}

VOID CPacketWriteStream::WriteINT64(ULONGLONG Value)
{
    *(ULONGLONG*)m_pData = Value;

    m_pData += 8;
    m_dwDataLen += 8;
}

VOID CPacketWriteStream::WriteBuffer(PVOID Value, DWORD Len)
{
    memcpy(m_pData, Value, Len);

    m_pData += Len;
    m_dwDataLen += Len;
}

IPacketBuffer* CPacketWriteStream::GetPacketBuffer()
{
    return CreateIBufferInstanceEx(m_pData, m_dwDataLen);
}

DWORD CPacketWriteStream::DataSize()
{
    return m_dwDataLen;
}