#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <tchar.h>

template<typename T> void WPM(uintptr_t address, T value) {
	try { *(T*)address = value; }
	catch (...) { return; }
}
template<typename Type>
bool WriteArray(HANDLE process, size_t addr, const Type* values, size_t count)
{
	SIZE_T size = count * sizeof(Type);
	DWORD old_prot = 0;
	if (!::VirtualProtectEx(process, (LPVOID)addr, size, PAGE_EXECUTE_READWRITE, &old_prot))
		return false;
	SIZE_T written = 0;
	::WriteProcessMemory(process, (LPVOID)addr, values, size, &written);
	if (!::VirtualProtectEx(process, (LPVOID)addr, size, old_prot, &old_prot))
		return false;
	return written == size;
}


DWORD GetPointerAddress(DWORD ptr, std::vector<DWORD> offsets)
{
	DWORD addr = ptr;
	for (int i = 0; i < offsets.size(); ++i)
	{
		addr = *(DWORD*)addr;
		addr += offsets[i];
	}
	return addr;
}

void WriteBytes(DWORD dwAddress, BYTE* pbBytes, int iCount)
{
	DWORD dwOld;
	VirtualProtect((void*)(dwAddress), iCount, 0x40, &dwOld);
	for (int i = 0; i < iCount; i++)
		*(BYTE*)(dwAddress + i) = pbBytes[i];
	VirtualProtect((void*)(dwAddress), iCount, dwOld, &dwOld);
}

void RedirectIOToConsole() {
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE* file;
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 500;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	freopen_s(&file, "CONOUT$", "w", stdout);
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
	size_t totalSize = size * nmemb;
	output->append((char*)contents, totalSize);
	return totalSize;
}