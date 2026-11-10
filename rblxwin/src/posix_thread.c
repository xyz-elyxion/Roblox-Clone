//========================================================================
// RBLXWIN 3.3 POSIX - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2017 Camilla Löwy <elmindreda@rblxwin.org>
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
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <assert.h>
#include <string.h>


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

RBLXWINbool _rblxwinPlatformCreateTls(_RBLXWINtls* tls)
{
    assert(tls->posix.allocated == RBLXWIN_FALSE);

    if (pthread_key_create(&tls->posix.key, NULL) != 0)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "POSIX: Failed to create context TLS");
        return RBLXWIN_FALSE;
    }

    tls->posix.allocated = RBLXWIN_TRUE;
    return RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyTls(_RBLXWINtls* tls)
{
    if (tls->posix.allocated)
        pthread_key_delete(tls->posix.key);
    memset(tls, 0, sizeof(_RBLXWINtls));
}

void* _rblxwinPlatformGetTls(_RBLXWINtls* tls)
{
    assert(tls->posix.allocated == RBLXWIN_TRUE);
    return pthread_getspecific(tls->posix.key);
}

void _rblxwinPlatformSetTls(_RBLXWINtls* tls, void* value)
{
    assert(tls->posix.allocated == RBLXWIN_TRUE);
    pthread_setspecific(tls->posix.key, value);
}

RBLXWINbool _rblxwinPlatformCreateMutex(_RBLXWINmutex* mutex)
{
    assert(mutex->posix.allocated == RBLXWIN_FALSE);

    if (pthread_mutex_init(&mutex->posix.handle, NULL) != 0)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR, "POSIX: Failed to create mutex");
        return RBLXWIN_FALSE;
    }

    return mutex->posix.allocated = RBLXWIN_TRUE;
}

void _rblxwinPlatformDestroyMutex(_RBLXWINmutex* mutex)
{
    if (mutex->posix.allocated)
        pthread_mutex_destroy(&mutex->posix.handle);
    memset(mutex, 0, sizeof(_RBLXWINmutex));
}

void _rblxwinPlatformLockMutex(_RBLXWINmutex* mutex)
{
    assert(mutex->posix.allocated == RBLXWIN_TRUE);
    pthread_mutex_lock(&mutex->posix.handle);
}

void _rblxwinPlatformUnlockMutex(_RBLXWINmutex* mutex)
{
    assert(mutex->posix.allocated == RBLXWIN_TRUE);
    pthread_mutex_unlock(&mutex->posix.handle);
}

