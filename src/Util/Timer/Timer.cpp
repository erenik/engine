// Emil Hedemalm
// 2013-06-10
// updated 2013-06-13 by Fredrik Larsson
// Adjustment/machineTime differentiation added 2014-02-10 by Emil

#include "Timer.h"
#include "Time/Time.h"
#include <iostream>

long long Timer::adjustment = 0;

Timer::Timer()
{
    running = false;
}

void Timer::Start()
{
	start = Time::Now();
	// Use time..
    running = true;
/*
    #ifdef WINDOWS
        // get ticks per second
        QueryPerformanceFrequency(&frequency);
        // start timer
        QueryPerformanceCounter(&t1);
        start = t1.QuadPart;
	#elif defined LINUX | defined OSX
        gettimeofday(&t1, NULL);
        start = t1.tv_sec;
	#endif
	*/
};

void Timer::Stop()
{
	stop = Time::Now();
    running = false;
/*
    #ifdef WINDOWS
        // stop timer
        QueryPerformanceCounter(&t2);
        stop = t2.QuadPart;
        // compute and print the elapsed time in millisec
        elapsedMicroseconds = (t2.QuadPart - t1.QuadPart) * 1000.0 * 1000.0 / frequency.QuadPart;
    //	std::cout<<"\nA* Time taken: "<<elapsedTime<<" milliseconds.";
    #elif defined LINUX | defined OSX
        gettimeofday(&t2, NULL);
        stop = t2.tv_sec;
        // compute time in millisec
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
    #endif
	*/
};

int64 Timer::GetMs()
{
	return GetMicros() / 1000;
	/*
	int64 milliseconds = 0;
    if (running)
    {
        #ifdef WINDOWS
            QueryPerformanceCounter(&t2);
            stop = t2.QuadPart;
            // compute and print the elapsed time in millisec
            elapsedMicroseconds = (t2.QuadPart - t1.QuadPart) * 1000.0 * 1000.0 / frequency.QuadPart;
        #elif defined LINUX | defined OSX
            gettimeofday(&t2, NULL);
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        #endif
    }
	milliseconds = elapsedMicroseconds / 1000.0;
	return milliseconds;
	*/
}


// Returns elapsed time in microseconds.
int64 Timer::GetMicros()
{
	if (stop.Type() == TimeType::UNDEFINED)
		stop = Time::Now();
	return (stop - start).Microseconds();
	/*
	int64 microSeconds;
    if (running)
    {
        #ifdef WINDOWS
            QueryPerformanceCounter(&t2);
            stop = t2.QuadPart;
            // compute and print the elapsed time in millisec
            elapsedMicroseconds = (t2.QuadPart - t1.QuadPart) * 1000.0 * 1000.0 / frequency.QuadPart;
        #elif defined LINUX | defined OSX
            gettimeofday(&t2, NULL);
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0 * 1000.0;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec);   // us to ms
        #endif
    }
	microSeconds = elapsedMicroseconds;
	return microSeconds;
	*/
}

// Returns time in seconds.
int64 Timer::GetCurrentTime()
{
	return GetCurrentTimeMs() * 0.001;
}


// Returns the current time. If true, returns machineTime, if false will return synchronized time as set with SetAdjustment.
time_t Timer::GetCurrentTimeMs(bool machineTime /* = true */)
{
#ifdef WINDOWS
	FILETIME ft_now;
	// Get current time in intervals of 100-nanoseconds from Jan 1, 1601
	GetSystemTimeAsFileTime(&ft_now);
	// Convert it to a 64 integer
//	__int64 currentTime = ft_now.dwLowDateTime + ((time_t)(ft_now.dwHighDateTime) << 32);
	time_t currTime = (time_t)ft_now.dwLowDateTime + ((time_t)(ft_now.dwHighDateTime) << 32);
	// Convert to UNIX epoch, Jan 1, 1970
	currTime -= (time_t)116444736000000000;
	// Convert the interval of 100-nanoseconds to milliseconds
	currTime /= 10000;
	currTime += adjustment;
	return currTime;
#elif defined LINUX | defined OSX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long currTime = (time_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
	currTime += adjustment;
    return currTime;
#endif
}

// Returns current time in microseconds.
time_t Timer::GetCurrentTimeMicro()
{
#ifdef WINDOWS
	FILETIME ft_now;
	// Get current time in intervals of 100-nanoseconds from Jan 1, 1601
	GetSystemTimeAsFileTime(&ft_now);
	// Convert it to a 64 integer
//	__int64 currentTime = ft_now.dwLowDateTime + ((time_t)(ft_now.dwHighDateTime) << 32);
	time_t currTime = (time_t)ft_now.dwLowDateTime + ((time_t)(ft_now.dwHighDateTime) << 32);
	// Convert to UNIX epoch, Jan 1, 1970
	currTime -= (time_t)116444736000000000;
	// Convert the interval of 100-nanoseconds to milliseconds
	currTime /= 1000;
	currTime += adjustment;
	return currTime;
#elif defined LINUX | defined OSX
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long currTime = (time_t)tv.tv_sec * 1000 * 1000 + tv.tv_usec;
	currTime += adjustment;
    return currTime;
#endif
}


/// Sets time adjustment that will be added to every call of GetCurrentTimeMs() unless specifically told otherwise.
void Timer::SetAdjustment(long long newAdjustment){
	adjustment = newAdjustment;
}

long long Timer::GetAdjustment(){
	return adjustment;
}