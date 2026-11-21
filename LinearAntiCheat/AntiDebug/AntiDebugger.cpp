//By AlSch092 @github
#include "AntiDebugger.hpp"

/*
	StartAntiDebugThread - creates a new thread on `CheckForDebugger`
*/
void Debugger::AntiDebug::StartAntiDebugThread()
{
	if (this->GetSettings() != nullptr && !this->GetSettings()->bUseAntiDebugging)
	{
		Logger::logf(Info, "Anti-Debugger was disabled in settings, debugging will be allowed");
		return;
	}

	this->DetectionThread = std::make_unique<Thread>((LPTHREAD_START_ROUTINE)Debugger::AntiDebug::CheckForDebugger, (LPVOID)this, true, false);

	Logger::logf(Info, "Created Debugger detection thread with Id: %d", this->DetectionThread->GetId());
}

/*
	CheckForDebugger - Thread function which loops and checks for the presense of debuggers
*/
void Debugger::AntiDebug::CheckForDebugger(LPVOID AD)
{
	if (AD == nullptr)
	{
		Logger::logf(Err, "AntiDbg class was NULL @ CheckForDebugger");
		return;
	}

	Debugger::AntiDebug* AntiDbg = reinterpret_cast<Debugger::AntiDebug*>(AD);

	Logger::logf(Info, "Started Debugger detection thread");

	bool MonitoringDebugger = true;

	const int MonitorLoopDelayMS = 1000;

	while (MonitoringDebugger)
	{
		if (AntiDbg->DetectionThread->IsShutdownSignalled())
		{
			Logger::logf(Info, "Shutting down Debugger detection thread with Id: %d", AntiDbg->DetectionThread->GetId());
			break;
		}

		//make new thread to check all other threads, since we suspend threads and check their context
		HANDLE CheckHardwareRegistersThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_IsHardwareDebuggerPresent, (LPVOID)AntiDbg, 0, 0);

		if (CheckHardwareRegistersThread == INVALID_HANDLE_VALUE || CheckHardwareRegistersThread == NULL)
		{
			Logger::logf(Warning, "Failed to create new thread to call _IsHardwareDebuggerPresent: %d", GetLastError());
		}

		AntiDbg->RunDetectionFunctions();

		if (AntiDbg->IsDBK64DriverLoaded())
		{
			AntiDbg->EvidenceManager->AddFlagged(DetectionFlags::DEBUG_DBK64_DRIVER);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(MonitorLoopDelayMS)); //ease the CPU a bit
	}
}


/*
    _IsHardwareDebuggerPresent - suspends each other thread and reads its debug registers.
    A breakpoint that is enabled (DR7 bits 0-7) or one that fired (DR6 bits 0-3) is a debugger;
    the raw DR6/DR7 values are the system's own defaults on an ordinary thread and are not.
*/
void Debugger::AntiDebug::_IsHardwareDebuggerPresent(LPVOID AD)
{
	if (AD == nullptr)
	{
		Logger::logf(Err, "AntiDbg class was NULL @ _IsHardwareDebuggerPresent");
		return;
	}

	Debugger::AntiDebug* AntiDbg = reinterpret_cast<Debugger::AntiDebug*>(AD);

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
	{
		Logger::logf(Err, "Error: unable to create toolhelp snapshot: %d\n", GetLastError());
		return;
	}

	DWORD currentProcessID = GetCurrentProcessId();

	if (Thread32First(hThreadSnap, &te32))
	{
		do
		{
			if (te32.th32OwnerProcessID == currentProcessID && te32.th32ThreadID != GetCurrentThreadId())
			{
				HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);

				if (hThread == NULL)
				{
					Logger::logf(Warning, "Error: unable to OpenThread on thread with id %d\n", te32.th32ThreadID);
					continue;
				}

				SuspendThread(hThread);

				CONTEXT context = {};
				context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

				// Nothing between the suspend and the resume may log, allocate or take a
				// lock. Logger::log holds a mutex, and the thread suspended here can be
				// the one holding it, which freezes every thread in the process - the
				// client's window turns white and its log stops in the middle of a line,
				// with the monitoring and audio threads stopped too. GetThreadContext is a
				// kernel call that takes no user-mode lock, so it stays inside the window;
				// its failure is reported after the thread is running again.
				const BOOL readContext = GetThreadContext(hThread, &context);
				const DWORD contextError = readContext ? 0 : GetLastError();

				ResumeThread(hThread);

				if (!readContext)
				{
					CloseHandle(hThread);
					Logger::logf(Err, "GetThreadContext failed with: %d", contextError);
					continue;
				}

				// Dr6 and Dr7 are not zero on an ordinary thread - the system leaves its
				// own defaults in them - so their raw values say nothing about a debugger.
				// What does: at least one breakpoint enabled (DR7 bits 0-7), or one that
				// actually fired (DR6 bits 0-3). Reading the defaults as evidence flagged
				// every ordinary session as hardware debugging.
				const bool breakpointEnabled = (context.Dr7 & 0xFF) != 0;
				const bool breakpointHit = (context.Dr6 & 0xF) != 0;

				if (breakpointEnabled || breakpointHit)
				{
					Logger::logf(Detection, "Found at least one debug register enabled (hardware debugging)");

					AntiDbg->AddFlagged(DetectionFlags::DEBUG_HARDWARE_REGISTERS);

					if (!AntiDbg->EvidenceManager->AddFlagged(DetectionFlags::DEBUG_HARDWARE_REGISTERS))
					{ //optionally take further action, `Flag` will already log a warning
					}

					CloseHandle(hThread);
					CloseHandle(hThreadSnap);
					return;
				}

				CloseHandle(hThread);
			}
		} while (Thread32Next(hThreadSnap, &te32));
	}
	else
	{
		Logger::logf(Err, "Thread32First Failed: %d\n", GetLastError());
		return;
	}

	CloseHandle(hThreadSnap);
	return;
}

/*
	PreventWindowsDebuggers - experimental, patches over some common debugging routines
*/
bool Debugger::AntiDebug::PreventWindowsDebuggers()
{
	HMODULE ntdll = GetModuleHandleA("ntdll.dll");

	if (!ntdll)
	{
		Logger::logf(Err, "Failed to find ntdll.dll @ AntiDebug::PreventWindowsDebuggers");
		return false;
	}

	DWORD dwOldProt = 0;

	UINT64 DbgBreakpoint_Address = (UINT64)GetProcAddress(ntdll, "DbgBreakPoint");
	UINT64 DbgUiRemoteBreakin_Address = (UINT64)GetProcAddress(ntdll, "DbgUiRemoteBreakin");

	if (DbgBreakpoint_Address)
	{
		if (VirtualProtect((LPVOID)DbgBreakpoint_Address, 1, PAGE_EXECUTE_READWRITE, &dwOldProt))
		{
			__try
			{
				*(BYTE*)DbgBreakpoint_Address = 0xC3;
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				Logger::logf(Err, "Failed to patch over DbgBreakpoint @ AntiDebug::PreventWindowsDebuggers");
				return false;
			}

			VirtualProtect((LPVOID)DbgBreakpoint_Address, 1, dwOldProt, &dwOldProt);
		}
	}

	if (DbgUiRemoteBreakin_Address)
	{
		if (VirtualProtect((LPVOID)DbgUiRemoteBreakin_Address, 1, PAGE_EXECUTE_READWRITE, &dwOldProt))
		{
			__try
			{
				*(BYTE*)DbgUiRemoteBreakin_Address = 0xC3;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				Logger::logf(Err, "Failed to patch over DbgUiRemoteBreakin @ AntiDebug::PreventWindowsDebuggers");
				return false;
			}

			VirtualProtect((LPVOID)DbgUiRemoteBreakin_Address, 1, dwOldProt, &dwOldProt); //set back original protections
		}
	}

	return true;
}

/*
	HideThreadFromDebugger - hides `hThread` from windows debuggers by calling NtSetInformationThread
	returns `true` on success
*/
bool Debugger::AntiDebug::HideThreadFromDebugger(HANDLE hThread)
{
	typedef NTSTATUS(NTAPI* pNtSetInformationThread) (HANDLE, UINT, PVOID, ULONG);
	NTSTATUS Status;

	pNtSetInformationThread NtSetInformationThread = (pNtSetInformationThread)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSetInformationThread");

	if (NtSetInformationThread == NULL)
		return false;

	if (hThread == NULL)
		Status = NtSetInformationThread(GetCurrentThread(), 0x11, 0, 0);
	else
		Status = NtSetInformationThread(hThread, 0x11, 0, 0);

	return (Status == 0);
}

bool Debugger::AntiDebug::IsDBK64DriverLoaded()
{
	return Services::IsDriverRunning(this->DBK64Driver);
}

void Debugger::AntiDebug::HideAllThreadsFromDebugger()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Failed to create snapshot." << std::endl;
		return;
	}

	DWORD pid = GetCurrentProcessId();

	THREADENTRY32 te;
	te.dwSize = sizeof(THREADENTRY32);

	if (Thread32First(hSnapshot, &te))
	{
		do
		{
			if (te.th32OwnerProcessID == pid)
			{
				HANDLE hThread = OpenThread(THREAD_SET_INFORMATION | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);

				if (!hThread)
				{
					std::cerr << "Failed to open thread: " << GetLastError() << std::endl;
					continue;
				}

				if (!Debugger::AntiDebug::HideThreadFromDebugger(hThread)) //hide thread from debuggers, placing this in the TLS callback allows all threads to be hidden
				{
					Logger::logf(Warning, " Failed to hide thread from debugger @ TLSCallback: thread id %d\n", GetCurrentThreadId());
				}

				CloseHandle(hThread);
			}
		} while (Thread32Next(hSnapshot, &te));
	}
	else
	{
		std::cerr << "Failed to retrieve thread information." << std::endl;
	}

	CloseHandle(hSnapshot);
}