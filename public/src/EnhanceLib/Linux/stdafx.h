#pragma once

#ifndef __STDAFX_H__
#define __STDAFX_H__

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#ifndef IOS
#include <net/if_arp.h>
#endif
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include "winpr/synch.h"
#include "winpr/thread.h"
#include "winpr/file.h"
#include "winpr/tchar.h"
#include "winpr/interlocked.h"
#include <CTLogEx.h>

#define MODULE_NAME _T("EnhanceLib")



#endif
