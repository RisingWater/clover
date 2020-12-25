#include "stdafx.h"
#include "SocketClient.h"
#include "SocketHelper.h"

CSocketClient::CSocketClient(const CHAR *address, WORD port) : CSocketBase(NET_SOCKET)
{
    strncpy(m_szDstAddress, address, 128);
    m_dwDstPort = port;
}

#ifndef WIN32

CSocketClient::CSocketClient(const CHAR* FileName) : CSocketBase(FILE_SOCKET)
{
    m_szFileName = strdup(FileName);
}

#endif

CSocketClient::~CSocketClient()
{
}

#ifdef ANDROID
#ifndef SUN_LEN //In case they fix it down the road
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen((ptr)->sun_path))
#endif
#endif

BOOL WINAPI CSocketClient::Connect()
{
    if (m_eSocketType == NET_SOCKET)
    {
        struct sockaddr_in  DstAddress;
        struct sockaddr_in  SrcAddress;

        int Ret;
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock < 0)
        {
            return FALSE;
        }

        memset(&DstAddress, 0, sizeof(sockaddr_in));
        DstAddress.sin_family = AF_INET;
        DstAddress.sin_port = htons(m_dwDstPort);
        DstAddress.sin_addr.s_addr = inet_addr(m_szDstAddress);

        Ret = connect(sock, (struct sockaddr *)&DstAddress, sizeof(sockaddr_in));
        if (Ret < 0)
        {
            closesocket(sock);
            return FALSE;
        }

        m_socket = sock;

#ifdef WIN32
        int len = sizeof(struct sockaddr_in);
#else
        socklen_t len = sizeof(struct sockaddr_in);
#endif
        if (getpeername(m_socket, (struct sockaddr*)&SrcAddress, &len) >= 0)
        {
            m_dwSrcPort = SrcAddress.sin_port;
            strcpy(m_szSrcAddress, inet_ntoa(SrcAddress.sin_addr));
        }

        Open();

        return TRUE;
    }
    else
    {
#ifndef WIN32
        struct sockaddr_un address;
        int    sock;
        size_t addr_length;
        int flag;

        if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            return FALSE;
        }

        flag = fcntl(sock, F_GETFD, 0);
        fcntl(sock, F_SETFD, flag | FD_CLOEXEC);

        memset(&address, 0, sizeof(struct sockaddr_un));
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, m_szFileName);
        addr_length = SUN_LEN(&address);

        if (connect(sock, (struct sockaddr *) &address, addr_length))
        {
            closesocket(sock);
            return FALSE;
        }

        m_socket = sock;

        Open();

        return TRUE;
#else
        return FALSE;
#endif
    }
}