#include "stdafx.h"
#include "CloverComm.h"
#include "CloverLink.h"
#include "TCPServer.h"

CCloverComm::CCloverComm() : ICloverComm()
{
    std::string Temp;
	m_dwServerPort = 0;

	m_pTCPService = NULL;

	m_pfnAcceptProcess = NULL;
	m_pAcceptProcessParam = NULL;

	InitializeCriticalSection(&m_csLock);
}

CCloverComm::~CCloverComm()
{
	RegisterAcceptCallback(NULL, NULL);

    if (m_pTCPService)
    {
		m_pTCPService->Release();
		m_pTCPService = NULL;
    }

	DeleteCriticalSection(&m_csLock);
}

void CCloverComm::RegisterAcceptCallback(_CloverAcceptProcess Process, CBaseObject* param)
{
	CBaseObject* oldParam = NULL;

	EnterCriticalSection(&m_csLock);

	oldParam = m_pAcceptProcessParam;

	m_pAcceptProcessParam = param;

	if (m_pAcceptProcessParam)
	{
		m_pAcceptProcessParam->AddRef();
	}

	m_pfnAcceptProcess = Process;

	LeaveCriticalSection(&m_csLock);

	if (oldParam)
	{
		oldParam->Release();
	}
}

BOOL CCloverComm::Listen(WORD Port)
{
	m_dwServerPort = Port;
	m_pTCPService = new CTCPService(Port);
	m_pTCPService->RegisterSocketAcceptProcess(CCloverComm::SocketAcceptProcessDelegate, this);
	return m_pTCPService->Init();
}

void CCloverComm::Close()
{
	if (m_pTCPService)
	{
		m_pTCPService->RegisterSocketAcceptProcess(NULL, NULL);
		m_pTCPService->Done();
	}

	return;
}

BOOL CCloverComm::SocketAcceptProcessDelegate(SOCKET s, CBaseObject* pParam)
{
	BOOL Ret = FALSE;
	CCloverComm* Comm = dynamic_cast<CCloverComm*>(pParam);

	if (Comm)
	{
		Ret = Comm->SocketAcceptProcess(s);
	}

	return Ret;
}

BOOL CCloverComm::SocketAcceptProcess(SOCKET s)
{
    CCloverLink* pLink = new CCloverLink(s, m_dwServerPort);

	if (pLink)
	{
		EnterCriticalSection(&m_csLock);

		if (m_pfnAcceptProcess)
		{
			m_pfnAcceptProcess(pLink, m_pAcceptProcessParam);
		}

		LeaveCriticalSection(&m_csLock);

		pLink->Release();
	}

	return TRUE;
}

BOOL CCloverComm::Connect(CHAR* IPAddress, WORD Port, ICloverLink **xlk)
{
    if (IPAddress == NULL || Port == 0 || xlk == NULL)
    {
        return FALSE;
    }

	*xlk = NULL;
	m_dwServerPort = Port;

	CCloverLink* pLink = new CCloverLink(IPAddress, m_dwServerPort);

	if (pLink)
	{
		if (pLink->Connect())
		{
			pLink->AddRef();
			*xlk = pLink;
			return TRUE;
		}

		pLink->Close();
		pLink->Release();
	}

	return FALSE;
}