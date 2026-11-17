//========================================================================
// LINEARWIN 3.3 Win32 - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@linearwin.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#include "internal.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

LINEARWINbool _linearwinPlatformCreateTls(_LINEARWINtls* tls)
{
    assert(tls->win32.allocated == LINEARWIN_FALSE);

    tls->win32.index = TlsAlloc();
    if (tls->win32.index == TLS_OUT_OF_INDEXES)
    {
        _linearwinInputErrorWin32(LINEARWIN_PLATFORM_ERROR,
                             "Win32: Failed to allocate TLS index");
        return LINEARWIN_FALSE;
    }

    tls->win32.allocated = LINEARWIN_TRUE;
    return LINEARWIN_TRUE;
}

void _linearwinPlatformDestroyTls(_LINEARWINtls* tls)
{
    if (tls->win32.allocated)
        TlsFree(tls->win32.index);
    memset(tls, 0, sizeof(_LINEARWINtls));
}

void* _linearwinPlatformGetTls(_LINEARWINtls* tls)
{
    assert(tls->win32.allocated == LINEARWIN_TRUE);
    return TlsGetValue(tls->win32.index);
}

void _linearwinPlatformSetTls(_LINEARWINtls* tls, void* value)
{
    assert(tls->win32.allocated == LINEARWIN_TRUE);
    TlsSetValue(tls->win32.index, value);
}

LINEARWINbool _linearwinPlatformCreateMutex(_LINEARWINmutex* mutex)
{
    assert(mutex->win32.allocated == LINEARWIN_FALSE);
    InitializeCriticalSection(&mutex->win32.section);
    return mutex->win32.allocated = LINEARWIN_TRUE;
}

void _linearwinPlatformDestroyMutex(_LINEARWINmutex* mutex)
{
    if (mutex->win32.allocated)
        DeleteCriticalSection(&mutex->win32.section);
    memset(mutex, 0, sizeof(_LINEARWINmutex));
}

void _linearwinPlatformLockMutex(_LINEARWINmutex* mutex)
{
    assert(mutex->win32.allocated == LINEARWIN_TRUE);
    EnterCriticalSection(&mutex->win32.section);
}

void _linearwinPlatformUnlockMutex(_LINEARWINmutex* mutex)
{
    assert(mutex->win32.allocated == LINEARWIN_TRUE);
    LeaveCriticalSection(&mutex->win32.section);
}

