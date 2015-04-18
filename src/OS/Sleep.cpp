/// Emil Hedemalm
/// 2015-04-18
// l

#include "Sleep.h"
#include "OS.h"

#ifdef WIDNOWS
#include <Windows.h>
#elif defined LINUX
#include <unistd.h>
#endif

void SleepThread(int milliseconds)
{
#ifdef WINDOWS
	SleepThread(milliseconds);
#elif defined LINUX
	uSleepThread(ms*1000);
#endif	
}
