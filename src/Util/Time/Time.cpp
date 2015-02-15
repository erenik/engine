/// Emil Hedemalm
/// 2014-07-16
/// Time-class, made to handle time values based on some certain starting point in time.

#include "Time.h"
#include "OS/OS.h"
#include <cassert>
#include <fstream>

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
Time::Time(int type)
	: type(type)
{
	intervals = 0;
}
/// Time using a given type and starting-point. 
Time::Time(uint64 intervals, int type)
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


/** Formats. See .h for clarification.
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
#define TWO_DIGITS(x) if (x < 10) newString += "0" + String(x); else newString += String(x);
			case 'Y': newString += String(year);	break;
			case 'M': TWO_DIGITS(month); break;
			case 'D': TWO_DIGITS(day); break;
			case 'H': TWO_DIGITS(hour); break;
			case 'N': TWO_DIGITS(minute); break;
			case 'S': TWO_DIGITS(second); break;
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

/// Arithmetic operations
Time Time::operator -= (const Time & otherTime)
{
	if (type == 0)
		type = otherTime.type;
	assert(type == otherTime.type);
	intervals -= otherTime.intervals;
	return Time(*this);
}
Time Time::operator += (const Time & otherTime)
{
	if (type == 0)
		type = otherTime.type;
	assert(type == otherTime.type);
	intervals += otherTime.intervals;
	return Time(*this);
}

/// Comparison operators
/// Larger than, returns true if this time is larger (more recent) than the other time. False if not.
bool Time::operator > (const Time & otherTime)
{
	Time otherTimeTmp = otherTime;
	// Convert as necessary
	otherTimeTmp.ConvertTo(type);
	// Compare
	return intervals > otherTime.intervals;
}

/// Lesser than, returns true if this time is smaller (older) than the other time. False if not.
bool Time::operator < (const Time & otherTime)
{
	Time otherTimeTmp = otherTime;
	// Convert as necessary
	otherTimeTmp.ConvertTo(type);
	// Compare
	return intervals < otherTime.intervals;
}	

bool Time::operator == (const Time & otherTime)
{
	if (type == otherTime.type)
	{
		if (intervals == otherTime.intervals)
			return true;
		return false;
	}
	// Convert as necessary?
	if (type != otherTime.type)
	{
		if (type == TimeType::UNDEFINED ||
			otherTime.type == TimeType::UNDEFINED)
			// Any bad type? Not equal then.
			return false;

		//
		assert(false && "Convert as necessary.");

	}
	return false;
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

/// Converts this time into specified type.
void Time::ConvertTo(int toType)
{
	// Same type, skip.
	if (type == toType)
		return;
	switch(type)
	{
		// Converting from undefined.. just set intervals to 0.
		case TimeType::UNDEFINED:
			intervals = 0;
			break;
		default:
			switch(toType)
			{
				case TimeType::UNDEFINED:
					intervals = 0;
					break;
				default:
					assert(false && "Lacking conversion scheme");
			}
	}
	this->type = toType;
}

/// o.o
void Time::AddMs(int amount)
{
	float multiplier = 1.0;
	switch(type)
	{
		case TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601: 
			// 100 nano to 1 micro: 10, 1 micro  to 1 ms: 1000.
			multiplier = 10 * 1000; 
			break; 
		case TimeType::MILLISECONDS_NO_CALENDER: 
			multiplier = 1.0f; 
			break;
		default:
			assert(false);
	}
	intervals += amount * multiplier;
}

/// Sets the hour by adding intervals.
void Time::SetHour(int hour)
{
	// Get current hour?
	int currentHour = Hour();
	int intervalsPerHour = IntervalsPerHour();
	intervals += (hour - currentHour) * intervalsPerHour;
}

/// Sets the hour in current hour by adding or removing diff amount of intervals.
void Time::SetMinute(int minute)
{
	int currentMinute = Minute();
	int intervalsPerMinute = IntervalsPerMinute();
	intervals += (minute - currentMinute) * intervalsPerMinute;
}


// Current total in micro-seconds since the starting-point.
uint64 Time::Microseconds()
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
	assert(micro >= 0);
	return micro / 1000;
}	

/// Total time in seconds.
int64 Time::Seconds()
{
	int64 micro = Microseconds();
	assert(micro >= 0);
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

int Time::IntervalsPerHour()
{
	int intervalsPerHour;
	switch(type)
	{
		case TimeType::MILLISECONDS_NO_CALENDER:
			/// 1000 ms to s, 60 s to m, 60, m to h (1000 * 3600)
			intervalsPerHour = 1000 * 3600;
			break;
		default:
			assert(false);
	}
	return intervalsPerHour;
}

int Time::IntervalsPerMinute()
{
	int intervalsPerMinute;
	switch(type)
	{
		case TimeType::MILLISECONDS_NO_CALENDER:
			/// 1000 ms to s, 60 s to m, 60, m to h (1000 * 3600)
			intervalsPerMinute = 1000 * 60;
			break;
		default:
			assert(false);
	}
	return intervalsPerMinute;
}

	

/// File I/O
bool Time::WriteTo(std::fstream & stream)
{
	int version = 0;
	stream.write((char*)&version, sizeof(int));
	assert(type != 0);
	stream.write((char*)&type, sizeof(int));
	stream.write((char*)&intervals, sizeof(uint64));
	return true;
}

bool Time::ReadFrom(std::fstream & stream)
{
	int version;
	stream.read((char*)&version, sizeof(int));
	assert(version == 0);
	stream.read((char*)&type, sizeof(int));
	assert(type != 0);
	stream.read((char*)&intervals, sizeof(uint64));
	return true;
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
			year = sysTime.wYear;
			month = sysTime.wMonth;
			day = sysTime.wDay;
			hour = sysTime.wHour;
			minute = sysTime.wMinute;
			second = sysTime.wSecond;

			break;
		}
		case TimeType::MILLISECONDS_NO_CALENDER:
		{
			second = (intervals / 1000) % 60;
			minute = (intervals / (1000 * 60)) % 60;
			hour = (intervals / (1000 * 3600)) % 24;
			day = intervals / (1000 * 3600 * 24);
			break;
		}
		default:
			std::cout<<"\nBad data in Time. Why are you trying to extract stuff from it?";
			assert(false);

	}
}


// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724950%28v=vs.85%29.aspx