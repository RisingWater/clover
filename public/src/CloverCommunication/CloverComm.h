#ifndef __CLOVER_COMM_H__
#define __CLOVER_COMM_H__

#ifdef WIN32
#include <windows.h>
#else
#include <winpr/wtypes.h>
#endif

#include "CloverCommunication.h"
#include "Base/BaseObject.h"

class CTCPService;
class CCloverLink;


class CCloverComm : public ICloverComm
{
public:
    CCloverComm();
    virtual ~CCloverComm();

    virtual BOOL CLOVER_API Listen(WORD Port);
    virtual void CLOVER_API RegisterAcceptCallback(_CloverAcceptProcess Process, CBaseObject* param);
    virtual BOOL CLOVER_API Connect(CHAR* IPAddress, WORD Port, ICloverLink ** xlk);
	virtual void CLOVER_API Close();

private:
	static BOOL SocketAcceptProcessDelegate(SOCKET s, CBaseObject* pParam);
	BOOL SocketAcceptProcess(SOCKET s);

    WORD m_dwServerPort;

	_CloverAcceptProcess m_pfnAcceptProcess;
	CBaseObject* m_pAcceptProcessParam;
	
	CTCPService* m_pTCPService;

	CRITICAL_SECTION m_csLock;
};

#endif
