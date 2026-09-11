#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

// --- NTSTATUS / NTAPI Setup ---
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

typedef LONG NTSTATUS;
#define NTAPI __stdcall

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation = 0,
    ThreadQuerySetWin32StartAddress = 9
} THREADINFOCLASS;

typedef NTSTATUS(NTAPI* pfnNtQueryInformationThread)(
    HANDLE ThreadHandle,
    THREADINFOCLASS ThreadInformationClass,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
    );

inline pfnNtQueryInformationThread GetNtQueryInformationThread()
{
    static pfnNtQueryInformationThread fn = nullptr;
    if (!fn)
    {
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll)
        {
            fn = reinterpret_cast<pfnNtQueryInformationThread>(
                GetProcAddress(hNtdll, "NtQueryInformationThread"));
        }
    }
    return fn;
}

// --- Get base address of current process's main module ---
inline uintptr_t GetMainModuleBase()
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32 me32 = { sizeof(me32) };
    uintptr_t base = 0;
    if (Module32First(hSnap, &me32))
        base = reinterpret_cast<uintptr_t>(me32.modBaseAddr);

    CloseHandle(hSnap);
    return base;
}

// --- Kill threads with offset < 0x100000 (i.e., 5-digit offset threads) ---
inline bool TerminateSmallOffsetThreadsS()
{
    DWORD currentPID = GetCurrentProcessId();
    uintptr_t baseAddress = GetMainModuleBase();
    if (baseAddress == 0)
        return false;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    std::vector<DWORD> threadIDs;
    THREADENTRY32 te32 = { sizeof(THREADENTRY32) };

    if (Thread32First(hSnapshot, &te32)) {
        do {
            if (te32.th32OwnerProcessID == currentPID)
                threadIDs.push_back(te32.th32ThreadID);
        } while (Thread32Next(hSnapshot, &te32));
    }
    CloseHandle(hSnapshot);

    struct ThreadInfo { DWORD id; void* start; };
    std::vector<ThreadInfo> threads;

    auto NtQueryInformationThread = GetNtQueryInformationThread();
    if (!NtQueryInformationThread)
        return false;

    for (DWORD tid : threadIDs) {
        HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_TERMINATE, FALSE, tid);
        if (!hThread) continue;

        void* startAddr = nullptr;
        if (NT_SUCCESS(NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress,
            &startAddr, sizeof(startAddr), nullptr))) {
            threads.push_back({ tid, startAddr });
        }

        CloseHandle(hThread);
    }

    // Step: Kill all threads with offset < 0x100000 (i.e. 5-digit hex)
    bool anyKilled = false;

    for (const auto& t : threads) {
        uintptr_t offset = reinterpret_cast<uintptr_t>(t.start) - baseAddress;
        if (offset < 0x100000) {
            HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, t.id);
            if (hThread) {
                TerminateThread(hThread, 0);
                CloseHandle(hThread);
            }
            anyKilled = true;
        }
    }


    return true;
}

// NtQueryInformationThread types
using NtQueryInformationThread_t = NTSTATUS(NTAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG);

inline bool SuspendNtdllThreadsSkipTrainer()
{
    DWORD currentPID = GetCurrentProcessId();

    // Get ntdll.dll base + size
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) return false;

    MODULEINFO mi{};
    if (!GetModuleInformation(GetCurrentProcess(), hNtdll, &mi, sizeof(mi)))
        return false;

    uintptr_t ntdllBase = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
    uintptr_t ntdllEnd = ntdllBase + mi.SizeOfImage;

    // Get the HMODULE of the module that contains this function (i.e. the trainer module)
    HMODULE hThisModule = nullptr;
    if (!GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&SuspendNtdllThreadsSkipTrainer),
        &hThisModule))
    {
        // best-effort: if we can't get it, fallback to main exe module
        hThisModule = GetModuleHandle(nullptr);
    }

    // Snapshot all threads
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return false;

    THREADENTRY32 te32{ sizeof(THREADENTRY32) };
    std::vector<DWORD> threadIDs;
    if (Thread32First(hSnapshot, &te32)) {
        do {
            if (te32.th32OwnerProcessID == currentPID)
                threadIDs.push_back(te32.th32ThreadID);
        } while (Thread32Next(hSnapshot, &te32));
    }
    CloseHandle(hSnapshot);

    // Prepare NtQueryInformationThread
    auto NtQueryInformationThread =
        (NtQueryInformationThread_t)GetProcAddress(hNtdll, "NtQueryInformationThread");
    if (!NtQueryInformationThread)
        return false;

    constexpr ULONG ThreadQuerySetWin32StartAddress = 9;
    bool anySuspended = false;

    // Helper: resolve module owning an address (returns nullptr if not found)
    auto ModuleFromAddress = [&](uintptr_t addr) -> HMODULE {
        HMODULE modules[1024];
        DWORD needed = 0;
        if (!EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed))
            return nullptr;
        DWORD count = needed / sizeof(HMODULE);
        for (DWORD i = 0; i < count; ++i) {
            MODULEINFO mInfo;
            if (!GetModuleInformation(GetCurrentProcess(), modules[i], &mInfo, sizeof(mInfo)))
                continue;
            uintptr_t mb = reinterpret_cast<uintptr_t>(mInfo.lpBaseOfDll);
            uintptr_t me = mb + mInfo.SizeOfImage;
            if (addr >= mb && addr < me)
                return modules[i];
        }
        return nullptr;
        };

    for (DWORD tid : threadIDs)
    {
        HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME, FALSE, tid);
        if (!hThread) continue;

        void* startAddr = nullptr;
        NTSTATUS status = NtQueryInformationThread(
            hThread,
            ThreadQuerySetWin32StartAddress,
            &startAddr,
            sizeof(startAddr),
            nullptr
        );

        if (status == 0 && startAddr != nullptr) // STATUS_SUCCESS == 0
        {
            uintptr_t addr = reinterpret_cast<uintptr_t>(startAddr);

            // Only consider threads whose start is inside ntdll
            if (addr >= ntdllBase && addr < ntdllEnd)
            {
                // Find module that owns that start address
                HMODULE owner = ModuleFromAddress(addr);

                // If owner is our trainer module or main exe, SKIP suspending it
                if (owner == hThisModule || owner == GetModuleHandle(nullptr))
                {
                    // skip — this thread is "connected" to our trainer/exe
                }
                else
                {
                    // suspend the thread (best-effort)
                    SuspendThread(hThread);
                    anySuspended = true;
                }
            }
        }

        CloseHandle(hThread);
    }

    return anySuspended;
}