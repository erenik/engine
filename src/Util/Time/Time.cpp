/// Emil Hedemalm
/// 2014-07-16
/// Time-class, made to handle time values based on some certain starting point in time.

#include "Time.h"
#include "OS/OS.h"
#include <cassert>

#ifdef WINDOWS
#include "OS/WindowsIncludes.h"
#endif

/// Current time.
Time::Time()
{
	intervals = 0;
	type = 0;
}
/// Undefined time.
Time::Time(int intervals)
	: intervals(intervals)
{
	type = TimeType::UNDEFINED;
}
/// Time using a given type and starting-point. 
Time::Time(int intervals, int type)
	: intervals(intervals), type(type)
{
}

/// Returns current time in default time-type/-format.
Time Time::Now()
{
	Time newTime;
#ifdef WINDOWS
	newTime.type = TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601;
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724280%28v=vs.85%29.aspx
	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	FILETIME fileTime;
	SystemTimeToFileTime(&sysTime, &fileTime);
	ULARGE_INTEGER uli;
	uli.LowPart = fileTime.dwLowDateTime;
	uli.HighPart = fileTime.dwHighDateTime;
	newTime.intervals = uli.QuadPart;
#endif
	return newTime;
}


/** Valid format, where the following characters will be replaced as follows:
	h - hours in 2 digits
	m - minute in 2 digits
	s - seconds in 2 digits
	
	Fetches calender data implicitly on each call.
*/
String Time::ToString(String withFormat)
{
	FetchCalenderData();

	String newString;
	for (int i = 0; i < withFormat.Length(); ++i)
	{
		char c = withFormat.c_str()[i];
		switch(c)
		{
			case 'h':
			{
				if (hour < 10)
					newString += "0" + String::ToString(hour);
				else
					newString += String::ToString(hour);
				break;
			}
			case 'm':
			{
				if (minute < 10)
					newString += "0" + String::ToString(minute);
				else
					newString += String::ToString(minute);
				break;
			}
			case 's':
			{
				if (second < 10)
					newString += "0" + String::ToString(second);
				else
					newString += String::ToString(second);
				break;
			}
			default:
				newString += c;
		}
	}
	return newString;
}

	

/// Arithmetic operations
Time Time::operator - (const Time & otherTime) const 
{
	Time newTime;
	if (type != otherTime.type)
	{
		// Undefined behaviour.
		std::cout<<"\nTime - otherTime with different types. Undefined behaviour, setting intervals to 0.";
		newTime.intervals = 0;
		return newTime;
	}
	assert(type == otherTime.type);
	newTime.intervals = intervals - otherTime.intervals;
	newTime.type = type;
	return newTime;
}


// Conversion operators
Time::operator const int64 () const 
{
	return intervals;
}

Time::operator const int32 () const
{
	return intervals;
}

// Current total in micro-seconds since the starting-point.
int64 Time::Microseconds()
{
	switch(type)
	{
		case TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601:
			/// 100-nano sec to micro second conversion..
			return intervals / 10;
		default:
			std::cout<<"\nFetching milliseconds from time of undefined type. Returning 0.";
			return 0;
	}	
	return -1;
}

int64 Time::Milliseconds()
{
	int64 micro = Microseconds();
	return micro / 1000;
}	

/// Total time in seconds.
int64 Time::Seconds()
{
	int64 micro = Microseconds();
	// Micro (millionth-part) to second.
	return micro / 1000000;
}


int Time::Minute()
{
	FetchCalenderData();
	return minute;
}
int Time::Hour()
{
	FetchCalenderData();
	return hour;
}
	


/// Fetches calender data given the intervals and type defined now.
void Time::FetchCalenderData()
{
	switch(type)
	{
		case TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601:
		{
			// First to larger integer..
			ULARGE_INTEGER uli;
			uli.QuadPart = intervals;

			// Then to filetime...
			FILETIME fileTime;
			fileTime.dwLowDateTime = uli.LowPart;
			fileTime.dwHighDateTime = uli.HighPart;

			// Then to system time.
			/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724280%28v=vs.85%29.aspx
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&fileTime, &sysTime);
			SystemTimeToFileTime(&sysTime, &fileTime);
			
			// Copy stuff.
			day = sysTime.wDay;
			hour = sysTime.wHour;
			minute = sysTime.wMinute;
			second = sysTime.wSecond;

			break;
		}
		default:
			assert(false);

	}
}


// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724950%28v=vs.85%29.aspx