#define WIN32_LEAN_AND_MEAN
#include <thread>


#include <chrono>
#include <Windows.h>
#include <fontsub.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <Psapi.h>
#include <cstdint>






MODULEINFO GetModuleInfo(char* szModule) {
	MODULEINFO modinfo = { 0 };
	HMODULE hModule = GetModuleHandle(szModule);
	if (hModule == 0)
		return modinfo;

	GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
	return modinfo;

}



DWORD SigScan(char* module, char* pattern, char* mask) {
	MODULEINFO mInfo = GetModuleInfo(module);
	DWORD base = (DWORD)mInfo.lpBaseOfDll;
	DWORD size = (DWORD)mInfo.SizeOfImage;

	DWORD patternLength = (DWORD)strlen(mask);

	for (DWORD i = 0; i < size - patternLength; i++) {
		bool found = true;
		for (DWORD j = 0; j < patternLength; j++) {
			found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
		}

		if (found)
			return base + i;

	}
	return 0x0;
}

bool script(void* address, void* jmpFunc, int len)
{
	if (len < 5) return false;

	DWORD curProtection;
	VirtualProtect(address, len, PAGE_EXECUTE_READWRITE, &curProtection);

	memset(address, 0x90, len);

	DWORD relativeAddress = ((DWORD)jmpFunc - (DWORD)address) - 5;

	DWORD sampleAdd = relativeAddress;
	*(BYTE*)address = 0xE9;
	*(DWORD*)((DWORD)address + 1) = relativeAddress;

	DWORD temp;

	VirtualProtect(address, len, curProtection, &temp);


	return true;
}