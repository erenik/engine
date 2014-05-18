// Emil Hedemalm
// 2013-06-10
// updated 2013-06-13 by Fredrik Larsson
// Adjustment/machineTime differentiation added 2014-02-10 by Emil

#ifndef AEONIC_TIMER_H
#define AEONIC_TIMER_H

#include "../../OS/OS.h"

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
// Undefine windows macro...
#undef GetCurrentTime
#endif
#if defined LINUX | defined OSX
    #include <sys/time.h>
#endif

#include "System/DataTypes.h"


class Timer {
public:
    Timer();

	void Start();
	void Stop();
	// Returns time in milliseconds
	long GetMs();
	// Returns elapsed time in microseconds.
	int64 GetMicro();
	// Returns time in seconds.
	static int64 GetCurrentTime();
	// Returns the current time. If true, returns unmodified machine time, if false will return synchronized time as set with SetAdjustment.
	static time_t GetCurrentTimeMs(bool machineTime = false);
	// Returns current time in microseconds.
	static time_t GetCurrentTimeMicro();
	/// Sets time adjustment that will be added to every call of GetCurrentTimeMs() unless specifically told otherwise.
	static void SetAdjustment(long long adjustment);
	static long long GetAdjustment();
private:
	long start;
	long stop;
	static long long adjustment;

	// Elapsed time in microseconds.
	double elapsedTime;
	bool running;

    #ifdef WINDOWS
        LARGE_INTEGER frequency;        // ticks per second
        LARGE_INTEGER t1, t2;           // ticks
    #elif defined LINUX | defined OSX
        timeval t1, t2;                 // structure containing sec and micro sec
    #endif


};

#endif // AEONIC_TIMER_H
