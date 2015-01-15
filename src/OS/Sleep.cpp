#include "Sleep.h"

#ifdef WINDOWS
#include <Windows.h>
void Wait(int milliseconds)
{
	Sleep(milliseconds);
}
#endif
