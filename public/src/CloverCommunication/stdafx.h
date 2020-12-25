#ifndef __STDAFX_H__
#define __STDAFX_H__

#ifdef _WIN32
#include <Winsock2.h>
#include  <mstcpip.h>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#else
#include <winpr/wtypes.h>
#include <unistd.h>
#include <sys/types.h>	       /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/tcp.h>
#endif

#pragma warning(disable:4127)
#pragma warning(disable:4996)

#if  defined(IS_64BIT)
#define AtomicLong volatile int
#else
#define AtomicLong volatile long
#endif

#include "Log/LogEx.h"
#include <string.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(a,b) ( (a) > (b) ? (b) : (a) )
#endif
#ifndef MAX
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#endif

#endif
