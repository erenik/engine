/// Emil Hedemalm
/// 2014-07-01
/// For checking System/OS memory status

#include "Memory.h"
#include "OS/OS.h"


struct MemoryStatus 
{
	int64 freeBytes;
	// Percentage.
	int memoryLoad;
} memoryStatus;

#ifdef LINUX
	#include <unistd.h>
#elif defined WINDOWS
	#include <windows.h>
	MEMORYSTATUSEX memoryStatusWin32;
#endif




/// Returns amount of available bytes.
void GetMemoryStatus()
{
#ifdef LINUX
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
#elif defined WINDOWS
    memoryStatusWin32.dwLength = sizeof(memoryStatusWin32);
    GlobalMemoryStatusEx(&memoryStatusWin32);
	memoryStatus.freeBytes = memoryStatusWin32.ullAvailPhys;
	memoryStatus.memoryLoad = memoryStatusWin32.dwMemoryLoad;
#endif
}

int64 AvailableMemoryMB()
{
	GetMemoryStatus();
	int64 bytes = memoryStatus.freeBytes;
	int64 kb = bytes / 1024;
	int64 mb = kb / 1024;
	return mb; 
}


/// Returns memory load, expressed as a percentage integer.
int MemoryLoad()
{
	GetMemoryStatus();
	return memoryStatus.memoryLoad;
}
