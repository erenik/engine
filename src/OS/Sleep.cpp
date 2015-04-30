/// Emil Hedemalm
/// 2015-04-18
// l

#include "Sleep.h"
#include "OS.h"

#ifdef WINDOWS
	#include <Windows.h>
	#include "Time/Time.h"
	bool timeCapsSet = false;
	int period = 1; // Default aim for 1 ms period?
#elif defined LINUX
	#include <unistd.h>
#endif

void SetTimerResolution(int milliseconds)
{
#ifdef WINDOWS
	TIMECAPS timecaps;
	int bytes = sizeof(timecaps);
	MMRESULT result = timeGetDevCaps(&timecaps, bytes);
	assert(result == TIMERR_NOERROR);
	assert(period <= timecaps.wPeriodMin);
	std::cout<<"lall";
	result = timeBeginPeriod(period);
	if (result == TIMERR_NOERROR)
	{
		timeCapsSet = true;
	}
	assert(result == TIMERR_NOERROR);
#endif
}

/// Resets to default value. Call on exit.
void ResetTimerResolution()
{
#ifdef WINDOWS
	MMRESULT result = timeEndPeriod(period);
	assert(result == TIMERR_NOERROR);
#endif
}


void SleepThread(int milliseconds)
{
#ifdef WINDOWS
	/// High-resolution. Use own Time-based evaluation.
	if (milliseconds < 20 && !timeCapsSet)
	{
		Time now = Time::Now();
		while(true)
		{
			Time again = Time::Now();
			if ((again - now).Milliseconds() > milliseconds)
				return;
		}
	}
	// Win32 sleep function which tends to sleep either 0-1, or multiples of 16 ms....
	else
		Sleep(milliseconds);
#elif defined LINUX
	usleep(milliseconds*1000);
#endif	
}
