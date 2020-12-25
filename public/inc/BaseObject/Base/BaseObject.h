#pragma once

#ifndef __BASE_OBJECT_H__
#define __BASE_OBJECT_H__

#ifdef WIN32
#include <Windows.h>
#else
#include <winpr/wtypes.h>
#endif

#ifdef WIN32
#define DLL_BASEOBJECT_API __declspec(dllexport)
#else
#define DLL_BASEOBJECT_API __attribute__ ((visibility("default"))) 
#endif

class DLL_BASEOBJECT_API CBaseObject
{
public:
    CBaseObject() { m_lRef = 1; };
    virtual ~CBaseObject() {};

    virtual LONG WINAPI AddRef()
    {
        return InterlockedIncrement(&m_lRef);
    };

    virtual LONG WINAPI Release()
    {
        LONG Temp = InterlockedDecrement(&m_lRef);
        if (Temp == 0)
        {
            delete this;
        }

        return Temp;
    };

private:
    volatile LONG m_lRef;
};

#endif