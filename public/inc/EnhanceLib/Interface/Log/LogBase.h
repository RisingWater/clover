#ifndef __LOG_BASE_H__
#define __LOG_BASE_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#include <winpr/tchar.h>
#endif

#ifdef ANDROID
#include <android/log.h>
#endif

enum log_level_t
{
    LOG_LEVEL_FATAL = 1,
    LOG_LEVEL_ERROR = 2,
    LOG_LEVEL_WARN = 3,
    LOG_LEVEL_INFO = 4,
    LOG_LEVEL_DEBUG = 5,
    LOG_LEVEL_TRACE = 6,

    LOG_TYPE_DUMP = 100,
    LOG_TYPE_TMP = 101,
};

enum log_output_type_t
{
    LOG_OUTPUT_DISABLE = 0x0,
    LOG_OUTPUT_STDERR = 0x1,
    LOG_OUTPUT_DBGPORT = 0x2,
    LOG_OUTPUT_FILE = 0x3,
};

#define LOGBASE_INIT_FLAGS_MONITOR 0x1

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
    int WINAPI LogBase_initW(const wchar_t *config_file);
    int WINAPI LogBase_initExW(const wchar_t *config_file, int flags);
#endif

    int WINAPI LogBase_initA(const char *config_file);
    int WINAPI LogBase_initExA(const char *config_file, int flags);

#ifdef _UNICODE
#define LogBase_init LogBase_initW
#define LogBase_initEx LogBase_initExW
#else
#define LogBase_init LogBase_initA
#define LogBase_initEx LogBase_initExA
#endif

#ifdef _WIN32
    void WINAPI LogBase_SetOptStrW(const wchar_t *modulename,
        const wchar_t *filedname,
        const wchar_t *value);
#endif
    void WINAPI LogBase_SetOptStrA(const char *modulename,
        const char *filedname,
        const char *value);

#ifdef _UNICODE
#define LogBase_SetOptStr LogBase_SetOptStrW
#else
#define LogBase_SetOptStr LogBase_SetOptStrA
#endif

#ifdef _WIN32
    void WINAPI LogBase_SetOptIntW(const wchar_t *modulename,
        const wchar_t *filedname,
        int value);
#endif
    void WINAPI LogBase_SetOptIntA(const char *modulename,
        const char *filedname,
        int value);
#ifdef _UNICODE
#define LogBase_SetOptInt LogBase_SetOptIntW
#else
#define LogBase_SetOptInt LogBase_SetOptIntA
#endif

#define LogBase_setOutput(type) \
    LogBase_SetOptInt(_T("GLOBAL"), _T("log_output_type"), (log_output_type_t)(type));

#define LogBase_setLogFile(filename) \
    { LogBase_SetOptStr(_T("GLOBAL"), _T("log_output_filepath"), (filename));  \
      LogBase_SetOptInt(_T("GLOBAL"), _T("log_output_type"),     LOG_OUTPUT_FILE); }

#define LogBase_enableDump(bEnable) \
    LogBase_SetOptInt(_T("GLOBAL"), _T("log_dump"), bEnable);

#define LogBase_setLogLevel(level) \
    LogBase_SetOptInt(_T("GLOBAL"), _T("log_level"), (log_level_t)(level));

    void WINAPI LogBase_reload();

#ifdef _WIN32
    void WINAPI LogBase_printfW(const wchar_t *module_name,
		enum log_level_t loglevel,
        const char *function_name,
        const char *file_name,
        int line,
        const wchar_t *pszFormat, ...);
#endif
	void WINAPI LogBase_printfA(const char *module_name,
		enum  log_level_t loglevel,
		const char *function_name,
		const char *file_name,
		int line,
		const char *pszFormat, ...)
#ifdef WIN32
		;
#else
		__attribute__((__format__(__printf__, 6, 7)));
#endif

#ifdef _UNICODE
#define LogBase_printf LogBase_printfW
#else
#define LogBase_printf LogBase_printfA
#endif

#ifdef ANDROID
#define L_TRACE(...) __android_log_print(ANDROID_LOG_VERBOSE,MODULE_NAME,__VA_ARGS__);
#else
#define L_TRACE(...) LogBase_printf(MODULE_NAME, LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

#define L_TRACE_ENTER() L_TRACE(_T("Enter\n"))
#define L_TRACE_LEAVE() L_TRACE(_T("Leave\n"))
#define L_TRACE_EXIT() L_TRACE(_T("Exit\n"))

#ifdef ANDROID
#define L_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG,MODULE_NAME,__VA_ARGS__);
#else
#define L_DEBUG(...) LogBase_printf(MODULE_NAME, LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

#ifdef ANDROID
#define L_INFO(...) __android_log_print(ANDROID_LOG_INFO,MODULE_NAME,__VA_ARGS__);
#else
#define L_INFO(...)  LogBase_printf(MODULE_NAME, LOG_LEVEL_INFO,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif
       
#ifdef ANDROID
#define L_WARN(...) __android_log_print(ANDROID_LOG_WARN,MODULE_NAME,__VA_ARGS__);
#else
#define L_WARN(...)  LogBase_printf(MODULE_NAME, LOG_LEVEL_WARN,  __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif

#ifdef ANDROID
#define L_ERROR(...) __android_log_print(ANDROID_LOG_ERROR,MODULE_NAME,__VA_ARGS__);
#else
#define L_ERROR(...) LogBase_printf(MODULE_NAME, LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);
#endif
         
#define L_FATAL(...) LogBase_printf(MODULE_NAME, LOG_LEVEL_FATAL, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);

#define L_TEMP(...) LogBase_printf(MODULE_NAME, LOG_TYPE_TMP, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__);

#ifdef _WIN32
    void WINAPI LogBase_dumpW(const wchar_t *module_name, int bForce, const char *function_name, const char *file_name, int line,
        const void *buf, int len);
#endif
    void WINAPI LogBase_dumpA(const char *module_name, int bForce, const char *function_name, const char *file_name, int line,
        const void *buf, int len);
#ifdef _UNICODE
#define LogBase_dump LogBase_dumpW
#else
#define LogBase_dump LogBase_dumpA
#endif

#define L_DUMP(buf, len) LogBase_dump(MODULE_NAME, 0, __FUNCTION__, __FILE__, __LINE__, buf, len);

#define L_TMPDUMP(buf, len) LogBase_dump(MODULE_NAME, 1, __FUNCTION__, __FILE__, __LINE__, buf, len);

void WINAPI LogBase_done(void);

#ifdef __cplusplus
}
#endif

#endif
