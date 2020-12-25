#ifndef __CTLOGEX_OS_WIN_H__
#define __CTLOGEX_OS_WIN_H__

#include "stdafx.h"

typedef std::wstring tstring;

typedef struct _MYSYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    DWORD dwSecond;
    DWORD dwUs;
} MYSYSTEMTIME, *PMYSYSTEMTIME;

BOOL Log_OsInit(const TCHAR *confname, int flags);

void Log_OsDone();

tstring Log_OsGetFullPathNameRelativeExec(const TCHAR *filename);

BOOL Log_OsMoveFile(const TCHAR *orgfilename, const TCHAR *newfilename);

void Log_OsGetTime(MYSYSTEMTIME *time, BOOL b_performance_counter);

void Log_OsShowError(TCHAR *fmt, ...);

void Log_OsLock();

void Log_OsUnlock();

unsigned long Log_CurrentThreadID();

#endif
