//By AlSch092 @github
#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include "../Common/Logger.hpp"
#include "../Process/Process.hpp"

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#endif

namespace Handles
{
    // One open handle from the system handle table, in the form the scan works with. This is
    // our own record rather than the kernel's layout: GetHandles copies the kernel's fields
    // into it one at a time, so it can keep the bookkeeping member at the end without reading
    // past the end of the snapshot buffer.
    typedef struct _SYSTEM_HANDLE
    {
        ULONG ProcessId;
        BYTE ObjectTypeNumber;
        BYTE Flags;
        PVOID Object;
        ULONG_PTR Handle; //the handle value in the owning process, not an index into the table
        ACCESS_MASK GrantedAccess;
        BOOL ReferencingOurProcess; //custom addition to the structure, we fill this member in ::DetectOpenHandlesToProcess
    } SYSTEM_HANDLE, * PSYSTEM_HANDLE;

    std::vector<SYSTEM_HANDLE> GetHandles();
    std::vector<SYSTEM_HANDLE>  DetectOpenHandlesToProcess();

    bool DoesProcessHaveOpenHandleToUs(__in const DWORD pid, __in const std::vector <Handles::SYSTEM_HANDLE> handleList);

// WebApp.exe, Server.exe and Studio.exe are this project's own executables. The site
// launcher starts the client with CreateProcess and keeps the process handle it waits
// on, so every normal launch by the launcher leaves an open handle to the client; without
// these entries the handle scan reports the launcher as an intruder on a clean machine.
#ifdef _DEBUG //in addition to checking process names, we should check the file's certificate to ensure a whitelisted process can't be spoofed to obtain a whitelisted handle
    static const wchar_t* Whitelisted[] = { {L"conhost.exe"}, {L"devenv.exe"}, {L"VsDebugConsole.exe"}, {L"WebApp.exe"}, {L"Server.exe"}, {L"Studio.exe"} }; //on different people's PC's, different processes might be opening handles to our process. 
#else
    static const wchar_t* Whitelisted[] = { {L"conhost.exe"}, {L"WebApp.exe"}, {L"Server.exe"}, {L"Studio.exe"} }; //non-console version of this project will be different and conhost should not be considered whitelisted 
#endif
}
