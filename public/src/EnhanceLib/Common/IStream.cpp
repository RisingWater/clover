#include "Common/IStream.h"
#include "Common/Stream.h"

IPacketReadStream* WINAPI CreateIReadStreamInstance(IPacketBuffer* Buffer)
{
    return new CPacketReadStream(Buffer);
}

IPacketWriteStream* WINAPI CreateIWriteStreamInstance(DWORD len)
{
    return new CPacketWriteStream(len);
}