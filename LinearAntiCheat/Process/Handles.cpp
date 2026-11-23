//By AlSch092 @github
#include "Handles.hpp"
#include <map>

/*
    GetHandles - Returns a vector of SYSTEM_HANDLE representing all running handles on the system
*/
std::vector<Handles::SYSTEM_HANDLE> Handles::GetHandles()
{
    typedef enum _SYSTEM_INFORMATION_CLASS
    {
        SystemHandleInfo = 16
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

    ULONG bufferSize = 0x10000;
    PVOID buffer = nullptr;
    NTSTATUS status = 0;

    do 
    {
        buffer = malloc(bufferSize);

        if (!buffer) 
        {
            Logger::logf(Err, "Memory allocation failed @ Handles::GetHandles");
            return {};
        }

        status = NtQuerySystemInformation(SystemHandleInfo, buffer, bufferSize, &bufferSize);

        if (status == STATUS_INFO_LENGTH_MISMATCH) 
        {
            free(buffer);
            bufferSize *= 2;
        }
        else if (!(((NTSTATUS)(status)) >= 0))
        {
            Logger::logf(Err, "NtQuerySystemInformation failed @ Handles::GetHandles");
            free(buffer);
            return {};
        }
    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    PSYSTEM_HANDLE_INFORMATION handleInfo = (PSYSTEM_HANDLE_INFORMATION)buffer;

    std::vector<SYSTEM_HANDLE> handles;

	for (ULONG i = 0; i < handleInfo->HandleCount; i++)
	{
		if (handleInfo->Handles[i].ProcessId <= 4) //save some memory and cpu by skipping system process handles
		{
			continue;
		}

        //if (handleInfo->Handles[i].ObjectTypeNumber != 7) //optionally, also skip if its not a process handle (for example, obtained from OpenProcess). however we can't guarantee it will be 7 across diff windows versions
        //{
        //    continue;
        //}

		SYSTEM_HANDLE handle = handleInfo->Handles[i];
		handle.ReferencingOurProcess = false; //the snapshot has no such member; do not leave garbage there
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
static bool ObjectTypeIndexForProcess(const std::vector<Handles::SYSTEM_HANDLE>& handles, BYTE* typeIndex)
{
    const DWORD currentProcessId = GetCurrentProcessId();
    HANDLE self = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, currentProcessId);

    if (self == NULL)
    {
        return false;
    }

    // Handle values are a multiple of four and the snapshot stores the index, so the
    // entry for this handle is the one whose HandleValue is the value divided by four.
    const USHORT selfIndex = static_cast<USHORT>(reinterpret_cast<ULONG_PTR>(self) / 4);
    bool found = false;

    for (const auto& handle : handles)
    {
        if (handle.ProcessId == currentProcessId && handle.Handle == selfIndex)
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
    BYTE processTypeIndex = 0;
    const bool typeIndexKnown = ObjectTypeIndexForProcess(handles, &processTypeIndex);

    // One process handle per process for the whole pass, instead of one per entry.
    std::map<DWORD, HANDLE> processes;
    auto processHandleFor = [&processes](DWORD processId) -> HANDLE
    {
        auto cached = processes.find(processId);
        if (cached != processes.end())
        {
            return cached->second;
        }

        HANDLE opened = OpenProcess(PROCESS_DUP_HANDLE, FALSE, processId);
        processes[processId] = opened;
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

        // The snapshot stores the index of the entry, and a handle value is that index
        // multiplied by four. Duplicating the index itself asked for an unrelated handle in
        // that process, so the check could not tell a process handle from anything else and
        // reported whichever handle happened to sit at the index.
        HANDLE duplicatedHandle = INVALID_HANDLE_VALUE;
        if (!DuplicateHandle(processHandle,
                             reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(handle.Handle) * 4),
                             GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            continue; //the handle was closed between the snapshot and now
        }

        if (GetProcessId(duplicatedHandle) == currentProcessId)
        {
            std::wstring procName = Process::GetProcessName(handle.ProcessId);
            int size = sizeof(Handles::Whitelisted) / sizeof(UINT64);

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
                Logger::logf(Detection, "Handle %d from process %d is referencing our process.", handle.Handle, handle.ProcessId);
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
