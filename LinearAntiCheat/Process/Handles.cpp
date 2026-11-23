//By AlSch092 @github
#include "Handles.hpp"
#include <map>

/*
    The kernel's view of one entry in the system handle table.

    Read through NtQuerySystemInformation's SystemExtendedHandleInformation (0x40), not the
    older SystemHandleInformation (16). Both describe the same table, but the older class
    stores the process id and the handle value in 16 bits each, so a process holding more
    than 0xFFFF handles had its handle values truncated on the way out - and the scan then
    asked DuplicateHandle for a handle that does not exist in that process. An invalid handle
    reference is not a failed call here: with the strict handle check it is a fatal error
    that no handler can catch, which is enough to close the game with nothing in any log.
    The extended class reports both as full-width values.
*/
namespace
{
    struct SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
    {
        PVOID Object;
        ULONG_PTR ProcessId;
        ULONG_PTR HandleValue; //the handle value, read in the context of ProcessId
        ACCESS_MASK GrantedAccess;
        USHORT CreatorBackTraceIndex;
        USHORT ObjectTypeIndex;
        ULONG HandleAttributes;
        ULONG Reserved;
    };

    struct SYSTEM_HANDLE_INFORMATION_EX
    {
        ULONG_PTR HandleCount;
        ULONG_PTR Reserved;
        SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
    };

    // Copies one kernel entry into our own record. Field by field on purpose: our struct is
    // larger than the kernel's, because of the ReferencingOurProcess member, so copying it
    // whole would read past the end of the snapshot buffer on the last entry.
    void CopyEntry(__in const SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX& entry, __out Handles::SYSTEM_HANDLE* handle)
    {
        handle->ProcessId = static_cast<ULONG>(entry.ProcessId);
        handle->ObjectTypeNumber = static_cast<BYTE>(entry.ObjectTypeIndex);
        handle->Flags = static_cast<BYTE>(entry.HandleAttributes);
        handle->Object = entry.Object;
        handle->Handle = entry.HandleValue;
        handle->GrantedAccess = entry.GrantedAccess;
        handle->ReferencingOurProcess = FALSE; //the snapshot has no such member; do not leave garbage there
    }
}

/*
    GetHandles - Returns a vector of SYSTEM_HANDLE representing all running handles on the system
*/
std::vector<Handles::SYSTEM_HANDLE> Handles::GetHandles()
{
    typedef enum _SYSTEM_INFORMATION_CLASS
    {
        SystemExtendedHandleInfo = 0x40
    } SYSTEM_INFORMATION_CLASS;

    typedef NTSTATUS(NTAPI* NtQuerySystemInformationFunc)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");

	if (hNtDll == NULL)
	{
		Logger::logf(Err, "Failed to fetch ntdll module address @ IsMachineAllowingSelfSignedDrivers. Error code : % lu\n", GetLastError());
		return {};
	}

    NtQuerySystemInformationFunc NtQuerySystemInformation = (NtQuerySystemInformationFunc)GetProcAddress(hNtDll, "NtQuerySystemInformation");

    if (!NtQuerySystemInformation)
    {
        Logger::logf(Err, "Could not get NtQuerySystemInformation function address @ Handles::GetHandles");
        return {};
    }

    // The extended snapshot is about 40 bytes per handle on the machine, so a megabyte to
    // start with and the call's own answer for how much more it needed.
    ULONG bufferSize = 0x100000;
    PVOID buffer = nullptr;
    ULONG returned = 0;
    NTSTATUS status = 0;

    do 
    {
        buffer = malloc(bufferSize);

        if (!buffer) 
        {
            Logger::logf(Err, "Memory allocation failed @ Handles::GetHandles");
            return {};
        }

        returned = 0;
        status = NtQuerySystemInformation(SystemExtendedHandleInfo, buffer, bufferSize, &returned);

        if (status == STATUS_INFO_LENGTH_MISMATCH) 
        {
            free(buffer);
            buffer = nullptr;

            // The call reports the size it needs; double as well in case it reported nothing.
            bufferSize = returned > bufferSize ? returned : bufferSize * 2;
        }
        else if (!(((NTSTATUS)(status)) >= 0))
        {
            Logger::logf(Err, "NtQuerySystemInformation failed @ Handles::GetHandles");
            free(buffer);
            return {};
        }
    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    SYSTEM_HANDLE_INFORMATION_EX* handleInfo = (SYSTEM_HANDLE_INFORMATION_EX*)buffer;

    std::vector<SYSTEM_HANDLE> handles;

	for (ULONG_PTR i = 0; i < handleInfo->HandleCount; i++)
	{
		if (handleInfo->Handles[i].ProcessId <= 4u) //save some memory and cpu by skipping system process handles
		{
			continue;
		}

        //if (handleInfo->Handles[i].ObjectTypeIndex != 7) //optionally, also skip if its not a process handle (for example, obtained from OpenProcess). however we can't guarantee it will be 7 across diff windows versions
        //{
        //    continue;
        //}

		SYSTEM_HANDLE handle{};
		CopyEntry(handleInfo->Handles[i], &handle);
		handles.push_back(handle);
	}

    free(buffer);
    return handles;
}

/*
    ObjectTypeIndexForProcess - the object type index a process handle carries in the system handle snapshot

    Only a handle of type Process can be an open handle to this process, but the index is
    not the same on every Windows build, so it is read rather than assumed: a handle to this
    process is opened, and its own entry in the snapshot is a process handle by definition.
    Returns false when that entry cannot be found, in which case the caller keeps scanning
    every entry instead of guessing.
*/
static bool ObjectTypeIndexForProcess(const std::vector<Handles::SYSTEM_HANDLE>& handles, USHORT* typeIndex)
{
    const DWORD currentProcessId = GetCurrentProcessId();
    HANDLE self = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, currentProcessId);

    if (self == NULL)
    {
        return false;
    }

    // The snapshot reports the handle's value itself, so its entry for this handle is the one
    // whose value is the value OpenProcess returned.
    const ULONG_PTR selfHandle = reinterpret_cast<ULONG_PTR>(self);
    bool found = false;

    for (const auto& handle : handles)
    {
        if (handle.ProcessId == currentProcessId && handle.Handle == selfHandle)
        {
            *typeIndex = handle.ObjectTypeNumber;
            found = true;
            break;
        }
    }

    CloseHandle(self);
    return found;
}

/*
    DetectOpenHandlesToProcess - returns a vector of SYSTEM_HANDLE which represent open handles in other processes to our current process
    Can be used to detect OpenProcess , a bit expensive on CPU though since all system handles must be enumerated
*/
std::vector<Handles::SYSTEM_HANDLE> Handles::DetectOpenHandlesToProcess()
{
    DWORD currentProcessId = GetCurrentProcessId();
    auto handles = GetHandles();
    std::vector<Handles::SYSTEM_HANDLE> handlesTous;

    // Every entry in the snapshot used to be opened and duplicated, one OpenProcess and one
    // DuplicateHandle for each handle on the machine: hundreds of thousands of kernel calls
    // per pass, every five seconds, nearly all of them about files, events and sections that
    // cannot be a handle to this process. The type index keeps the pass to process handles.
    USHORT processTypeIndex = 0;
    const bool typeIndexKnown = ObjectTypeIndexForProcess(handles, &processTypeIndex);

    // One process handle per process for the whole pass, instead of one per entry.
    std::map<DWORD, HANDLE> processes;
    auto processHandleFor = [&processes](ULONG_PTR processId) -> HANDLE
    {
        const DWORD process = static_cast<DWORD>(processId);
        auto cached = processes.find(process);
        if (cached != processes.end())
        {
            return cached->second;
        }

        HANDLE opened = OpenProcess(PROCESS_DUP_HANDLE, FALSE, process);
        processes[process] = opened;
        return opened;
    };

    for (auto& handle : handles)
    {
        if (handle.ProcessId == currentProcessId || handle.ProcessId <= 4 || handle.Handle == 0)
        {
            continue; // our own handles, the kernel's, and the null handle can never be an open handle to us
        }

        if (typeIndexKnown && handle.ObjectTypeNumber != processTypeIndex)
        {
            continue;
        }

        HANDLE processHandle = processHandleFor(handle.ProcessId);

        if (processHandle == NULL) //possibly SYSTEM process, or one that has already exited
        {
            continue;
        }

        // The snapshot already carries the handle value, so it is passed as it came: turning
        // it into an index, or an index into it, asks the process about a handle it does not
        // have. The snapshot is a moment in time, so the value can also have been closed
        // since; that duplicate simply fails and the entry is skipped.
        HANDLE duplicatedHandle = INVALID_HANDLE_VALUE;
        if (!DuplicateHandle(processHandle,
                             reinterpret_cast<HANDLE>(handle.Handle),
                             GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            continue; //the handle was closed between the snapshot and now
        }

        if (GetProcessId(duplicatedHandle) == currentProcessId)
        {
            std::wstring procName = Process::GetProcessName(handle.ProcessId);
            int size = sizeof(Handles::Whitelisted) / sizeof(Handles::Whitelisted[0]);

            bool foundWhitelisted = false;

            for (int i = 0; i < size; i++)   // Check if handle belongs to whitelisted process; if so skip this handle
            {
                if (wcscmp(Handles::Whitelisted[i], procName.c_str()) == 0) //whitelisted program has open handle
                {
                    foundWhitelisted = true;
                    break;
                }
            }

            if (!foundWhitelisted)
            {
                Logger::logf(Detection, "Handle %llX from process %lu is referencing our process.",
                             static_cast<unsigned long long>(handle.Handle), handle.ProcessId);
                handle.ReferencingOurProcess = true;
                handlesTous.push_back(handle);
            }
        }

        CloseHandle(duplicatedHandle);
    }

    for (auto& entry : processes)
    {
        if (entry.second != NULL)
        {
            CloseHandle(entry.second);
        }
    }

    return handlesTous;
}

/*
    DoesProcessHaveOpenHandleTous - returns true if any handle in vector `handles`  points to our process
    Can be used to detect open process handles to our process from other processes
*/
bool Handles::DoesProcessHaveOpenHandleToUs(__in const DWORD pid, __in const std::vector <Handles::SYSTEM_HANDLE> handles)
{
    if (pid <= 4) //skip system-related processes
        return false;

    for (const auto& handle : handles)
    {
        if (handle.ProcessId == pid && handle.ReferencingOurProcess)
        {
            return true;
        }   
    }

    return false;
}
