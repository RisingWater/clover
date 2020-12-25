#include "Windows/IPipeClient.h"
#include "Windows/IPipeServer.h"
#include "Windows/PipeClient.h"
#include "Windows/PipeServer.h"

IPipeClient* WINAPI CreateIPipeClientInstance(TCHAR *PipeName, DWORD Timeout)
{
    return new CPipeClient(PipeName, Timeout);
}

IPipeServerService* WINAPI CreateIPipeServerServiceInstance(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent)
{
    return new CPipeServerService(PipeName, Timeout, StopEvent);
}

#if 0
IBasePipeClient* WINAPI CreateIBasePipeClientInstance(TCHAR *PipeName, DWORD Timeout)
{
    return new CBasePipeClient(PipeName, Timeout);
}

IBasePipeServerService* WINAPI CreateIBasePipeServerServiceInstance(TCHAR* PipeName, DWORD Timeout, HANDLE StopEvent)
{
    return new CBasePipeServerService(PipeName, Timeout, StopEvent);
}
#endif