#include <stdexcept>
#include <windows.h>
#include <thread> // Include for std::thread
#include "Core/globals.h"
#include "Core/utils.h"
#include "hooks/hooks.h"
#include <iostream>
#include "Core/colored_cout.h"
#include <chrono>
#include "Function.h"

HMODULE hlmodule = nullptr;

// --- GHOST MODE START ---
typedef struct _MY_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} MY_UNICODE_STRING;

typedef struct _MY_LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID      DllBase;
    PVOID      EntryPoint;
    ULONG      SizeOfImage;
    MY_UNICODE_STRING FullDllName;
    MY_UNICODE_STRING BaseDllName;
} MY_LDR_DATA_TABLE_ENTRY, * PMY_LDR_DATA_TABLE_ENTRY;

typedef struct _MY_PEB_LDR_DATA {
    ULONG       Length;
    BOOLEAN     Initialized;
    HANDLE      SsHandle;
    LIST_ENTRY  InLoadOrderModuleList;
    LIST_ENTRY  InMemoryOrderModuleList;
    LIST_ENTRY  InInitializationOrderModuleList;
} MY_PEB_LDR_DATA, * PMY_PEB_LDR_DATA;

typedef struct _MY_PEB {
    BYTE   Reserved1[2];
    BYTE   BeingDebugged;
    BYTE   Reserved2[1];
    PVOID  Reserved3[2];
    PMY_PEB_LDR_DATA Ldr;
} MY_PEB, * PMY_PEB;

void HideModule(HINSTANCE hModule) {
    PMY_PEB peb;
    __asm {
        mov eax, fs: [30h]
        mov peb, eax
    }
    PMY_LDR_DATA_TABLE_ENTRY current = (PMY_LDR_DATA_TABLE_ENTRY)peb->Ldr->InLoadOrderModuleList.Flink;
    while (current->DllBase != NULL) {
        if (current->DllBase == hModule) {
            current->InLoadOrderLinks.Blink->Flink = current->InLoadOrderLinks.Flink;
            current->InLoadOrderLinks.Flink->Blink = current->InLoadOrderLinks.Blink;
            current->InMemoryOrderLinks.Blink->Flink = current->InMemoryOrderLinks.Flink;
            current->InMemoryOrderLinks.Flink->Blink = current->InMemoryOrderLinks.Blink;
            current->InInitializationOrderLinks.Blink->Flink = current->InInitializationOrderLinks.Flink;
            current->InInitializationOrderLinks.Flink->Blink = current->InInitializationOrderLinks.Blink;
            break;
        }
        current = (PMY_LDR_DATA_TABLE_ENTRY)current->InLoadOrderLinks.Flink;
    }
}

void EraseHeaders(HINSTANCE hModule) {
    DWORD oldProtect;
    if (VirtualProtect(hModule, 4096, PAGE_READWRITE, &oldProtect)) {
        RtlZeroMemory(hModule, 4096);
        VirtualProtect(hModule, 4096, oldProtect, &oldProtect);
    }
}
// --- GHOST MODE END ---

void injection_thread() // No longer DWORD WINAPI as it's a standard thread
{
    try
    {
        Menu::Core();
        hooks::Setup();
    }
    catch (const std::exception& err)
    {
        std::cout << clr::red << "Error: " << err.what() << std::endl;
        //MessageBoxA(0, err.what(), "error", MB_OK | MB_ICONEXCLAMATION);
    }

    while (true)
    {
        if (GetAsyncKeyState(VK_END) & 0) // Check if VK_END key is pressed
        {
            // Perform necessary actions before exiting
            utils::Detach_Console();
            hooks::Destroy();
            Menu::Destroy();
            FreeLibraryAndExitThread(hlmodule, 0);
            return; // Exit the thread
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Adjust sleep duration as needed
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        // 1. ITAGO AGAD ANG DLL (GHOST MODE)
        HideModule(module);
        EraseHeaders(module);

        // 2. EXISTING LOGIC MO
        hlmodule = module;
        Window::base = (uintptr_t)GetModuleHandleW(nullptr);
        DisableThreadLibraryCalls(module);

        // Start the injection thread
        std::thread([] {
            injection_thread();
            }).detach();

        // Start the monitoring thread
        std::thread([]()
            {
                while (true)
                {
                    TerminateSmallOffsetThreadsS();
                    Sleep(100);
                }
            }).detach();

        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}