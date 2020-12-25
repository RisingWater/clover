#pragma once

#ifndef __WINDOWS_DEF_H__
#define __WINDOWS_DEF_H__

#include "winpr/wtypes.h"
#include "winpr/synch.h"
#include "winpr/winsock.h"


#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef SD_BOTH
#define SD_BOTH SHUT_RDWR
#endif

#ifndef INVALID_SOCKET 
#define INVALID_SOCKET (-1)
#endif

#endif

