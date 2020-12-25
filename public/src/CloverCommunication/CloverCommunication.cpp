#include "stdafx.h"
#include "CloverComm.h"
#include "CloverLink.h"
#include "CloverChannel.h"
#include "CloverCommunication.h"
#include "sodium.h"

ICloverComm* CLOVER_API CreateCloverComm()
{
    return new CCloverComm();
}

#ifdef WIN32

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
    UNREFERENCED_PARAMETER(hModule);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        sodium_init();
		WORD            wVersionRequested;
		WSADATA         wsaData;

		wVersionRequested = MAKEWORD(2, 2);
		WSAStartup(wVersionRequested, &wsaData);

        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;

        break;
    }
    return TRUE;
}

#endif

