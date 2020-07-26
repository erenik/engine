/// Emil Hedemalm
/// 2014-07-16
/// Time-class, made to handle time values based on some certain starting point in time.

#include "Time.h"
#include "OS/OS.h"
#include <cassert>
#include <fstream>

#include "OS/Sleep.h"

#ifdef WINDOWS
	#include "OS/WindowsIncludes.h"
#elif defined LINUX
	#include <sys/time.h>
#endif

/// Dicates default type of created Time objects. Default is TimeType::UNDEFINED
TimeType Time::defaultType = TimeType::UNDEFINED;

/// Current time.
Time::Time()
{
	intervals = 0;
	type = defaultType;
}
/// Undefined time.
Time::Time(TimeType type)
	: type(type)
{
	if (type == TimeType::UNDEFINED)
		std::cout<<"\nWARNING: Time of undefined type created.";
//	assert(type > TimeType::UNDEFINED && type < TimeType::TIME_TYPES);
	intervals = 0;
}
/// Time using a given type and starting-point. 
Time::Time(TimeType type, uint64 intervals)
	: intervals(intervals), type(type)
{
	assert(type > TimeType::UNDEFINED && type < TimeType::TIME_TYPES);
}

/// Unit test.
void Time::Test()
{
	int sleepTime = 10;
	while(true)
	{
		SleepThread(sleepTime);
		Time now = Time::Now();
		std::cout<<"\n"+now.ToString("Y-M-D H:m:S");
		++sleepTime;
	}
}

/// o.o
Time Time::Milliseconds(int amount)
{
	Time time;
	time.type = TimeType::MILLISECONDS_NO_CALENDER;
	time.intervals = amount;
	return time;
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
#elif defined LINUX
	// Old version, marked as deprecated in POSIX 2008: http://man7.org/linux/man-pages/man2/settimeofday.2.html
	/*
	timeval t1;
    gettimeofday(&t1, NULL);
    newTime.type = TimeType::LINUX_MICROSEC_SINCE_JAN1_1970;
    // seconds + micro seconds
    newTime.intervals = t1.tv_sec * 1000000 + t1.tv_usec;
    std::cout<<"\ngtod secs: "<<t1.tv_sec;
    */
    // End result: microseconds since Epoch.

    // New version: http://man7.org/linux/man-pages/man2/clock_gettime.2.html
    timespec resolution, time;
    clock_getres(CLOCK_REALTIME, &resolution);
  //  std::cout<<"\nResolution seconds:"<<resolution.tv_sec<<" nanosec: "<<resolution.tv_nsec;
    clock_gettime(CLOCK_REALTIME, &time);
  //  std::cout<<"\nTime: seconds:"<<time.tv_sec<<" nsec"<<time.tv_nsec;
  //  std::cout<<"\n Realtime: "<<CLOCK_REALTIME;

    newTime.intervals = time.tv_sec;
    newTime.intervals *= 1000000;
    newTime.intervals += time.tv_nsec / 1000;
	newTime.type = TimeType::LINUX_MICROSEC_SINCE_JAN1_1970;
    
//	std::cout<<"\nIntervals: "<<newTime.intervals;
#endif
	return newTime;
}


/** Formats. See .h for clarification.
*/
String Time::ToString(String withFormat)
{
	FetchCalenderData();

	String newString;
	int negater = 1;
	for (int i = 0; i < withFormat.Length(); ++i)
	{
		char c = withFormat.c_str()[i];
		switch(c)
		{
#define TWO_DIGITS(x) \
			negater = 1;\
			if (x < 0){\
				newString += "-"; negater = -1;\
			}\
			if (x < 10) \
				newString += "0" + String(negater * x); \
			else if (x == 0)\
				newString = "00";\
			else \
				newString += String(x * negater);
			case 'Y': newString += String(year);	break;
			case 'M': TWO_DIGITS(month); break;
			case 'D': TWO_DIGITS(day); break;
			case 'H': TWO_DIGITS(hour); break;
			case 'm': TWO_DIGITS(minute); break;
			case 'S': TWO_DIGITS(second); break;
			case 'n': TWO_DIGITS(millisecond); break;
			default:
				newString += c;
		}
	}
	return newString;
}


#define ASSERT_SAME_TYPE if (type != otherTime.type)\
{ 		\
	std::cout<<"\nTime - otherTime with different types. Undefined behaviour, setting intervals to 0."; newTime.intervals = 0;\
	assert(false && "Bad types.");\
	return newTime;\
}
#define ASSERT_SAME_TYPE //


/// Arithmetic operations
Time Time::operator - (const Time & otherTime) const 
{
	Time newTime;
	ASSERT_SAME_TYPE;
	if (type != otherTime.type)
		std::cout<<"\nNot same type D:";
	assert(type == otherTime.type);
	newTime.intervals = intervals - otherTime.intervals;
	newTime.type = type;
	return newTime;
}

Time Time::operator + (const Time & otherTime) const
{
	Time newTime;
	if (type != otherTime.type)
	{
		// Try convert.
		int types[TimeType::TIME_TYPES];
		types[type]++;
		types[otherTime.type]++;
		// If one is windows - convert to it.
		if (types[TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601])
		{
			Time t1 = *this, t2 = otherTime;
			t1.ConvertTo(TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601);
			t2.ConvertTo(TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601);
			return t1 + t2;
		}
		return newTime;
	}
	ASSERT_SAME_TYPE;
	newTime.intervals = intervals + otherTime.intervals;
	newTime.type = type;
	return newTime;
}

/// Arithmetics.
Time Time::operator * (const float & f) const
{
	assert(type == TimeType::MILLISECONDS_NO_CALENDER);
	Time newTime = *this;
	newTime.intervals *= f;
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
			otherTime.type == TimeType::UNDEFINED ||
			type == TimeType::MILLISECONDS_NO_CALENDER ||
			otherTime.type == TimeType::MILLISECONDS_NO_CALENDER)
		{
			// Any bad type? Not equal then.
			std::cout<<"\nComparng incomparable time-types.";
			return false;
		}
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
void Time::ConvertTo(TimeType toType)
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
		case TimeType::MILLISECONDS_NO_CALENDER:
		{
			/// From millisec to microsec -> 1000x, microsec to 100 nanosec -> 10x
			if (toType == TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601)
				intervals *= 10 * 1000;
			else 
				assert(false);
			break;
		}
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

void Time::AddSeconds(int seconds) {
	AddMs(seconds * 1000);
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

void Time::SetSecond(int second)
{
	int currentSecond = Second();
	int intervalsPerSecond = IntervalsPerSecond();
	intervals += (second - currentSecond) * intervalsPerSecond;
}

void Time::SetMillisecond(int newM)
{
	int currentMilli = Millisecond();
	int intervalsPerMillisecond = IntervalsPerMillisecond();
	intervals += (newM - currentMilli) * intervalsPerMillisecond;
}

// Current total in micro-seconds since the starting-point.
uint64 Time::Microseconds() const
{
	switch(type)
	{
		case TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601:
			/// 100-nano sec to micro second conversion..
			return intervals / 10;
		case TimeType::MILLISECONDS_NO_CALENDER:
			return intervals * 1000;
		case TimeType::LINUX_MICROSEC_SINCE_JAN1_1970:
			return intervals;
		default:
			std::cout<<"\nFetching milliseconds from time of undefined type. Returning 0.";
			return 0;
	}	
	return -1;
}

int64 Time::Milliseconds() const
{
	switch(type)
	{
		case TimeType::MILLISECONDS_NO_CALENDER:
			return intervals;
	}
	int64 micro = Microseconds();
//	assert(micro >= 0);
	return micro / 1000;
}	

/// Total time in seconds.
int64 Time::Seconds() const
{
	int64 micro = Microseconds();
	assert(micro >= 0);
	// Micro (millionth-part) to second.
	return micro / 1000000;
}

int64 Time::Minutes() const
{
	return Seconds() / 60;
}

int Time::Millisecond() // 0 to 999
{
	FetchCalenderData();
	return millisecond;
}	
int Time::Second()
{
	FetchCalenderData();
	return second;
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

int Time::IntervalsPerSecond()
{
	int intervalsPerSecond;
	switch(type)
	{
		case TimeType::MILLISECONDS_NO_CALENDER:
			intervalsPerSecond = 1000;
			break;
		default:
			assert(false);
	}
	return intervalsPerSecond;
}	

int Time::IntervalsPerMillisecond()
{
	int intervalsPerMs;
	switch(type)
	{
		case TimeType::MILLISECONDS_NO_CALENDER:
			intervalsPerMs = 1;
			break;
		default:
			assert(false);
	}
	return intervalsPerMs;	
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

/// String I/O
bool Time::ParseFrom(const String & string)
{
	/// Any formatting?
	List<String> tokens = string.Tokenize(":.");
	if (tokens.Size() == 1) { // Set seconds only.
		this->SetSecond(tokens[0].ParseInt());
	}
	else if (tokens.Size() == 2)
	{
		// Minute-second, by default?
		if (string.Contains(":"))
		{
			this->SetMinute(tokens[0].ParseInt());
			this->SetSecond(tokens[1].ParseInt());
		}
		else if (string.Contains("."))
		{
			this->SetSecond(tokens[0].ParseInt());
			String milliStr = (tokens[1] + "0000").Part(0,3);
			this->SetMillisecond(milliStr.ParseInt());		
		}
		return true;
	}
	else if (tokens.Size() == 3)
	{
		this->SetMinute(tokens[0].ParseInt());
		this->SetSecond(tokens[1].ParseInt());
		String milliStr = (tokens[2] + "0000").Part(0,3);
		this->SetMillisecond(milliStr.ParseInt());		
		return true;
	}
	return false;
}

Time ParseTimeFrom(const String & string) {
	Time time = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);
	time.ParseFrom(string);
	return time;
}


/// Prints type, intervals, and relevant unit
void Time::PrintData()
{
//	FetchCalenderData();
	std::cout<<"\nType: "<<type<<" intervals: "<<intervals<<" seconds: "<<Seconds();
}

/// Fetches calender data given the intervals and type defined now.
void Time::FetchCalenderData()
{
	switch(type)
	{
		case TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601:
		{
#ifdef WINDOWS
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
			millisecond = sysTime.wMilliseconds;
#else 	// Non-Windows? Convert it.
			assert(false);
#endif // WINDOWS
			break;
		}
		case TimeType::LINUX_MICROSEC_SINCE_JAN1_1970:
		{
#ifdef LINUX
			year = month = day = hour = minute = second = -1;
//			strftime(buf, sizeof(buf), "");
			// Use Date function for simplicity...
			// Ref: http://man7.org/linux/man-pages/man1/date.1.html
//			year = 

			
	//		std::cout<<"\nIntervals: "<<intervals;
			int64 seconds = intervals / 1000000;
	//		std::cout<<"\nSeconds: "<<seconds;
			time_t tt = seconds;
			tm t;
			tm * reuslt = localtime_r(&tt, &t);
			second = seconds % 60;
			second = t.tm_sec;
			minute = t.tm_min;
			hour = t.tm_hour;
			day = t.tm_mday;
			month =  t.tm_mon + 1; // Months since january
			year = t.tm_year + 1900; // Years since 1900
#else
			assert(false);
#endif
	/*		int64 seconds = intervals / 1000000;
			int minutes = seconds / 60;
			int hours = minutes / 60;
			int days = hours / 24;
			int years = days / 365.25;

			second = seconds % 60;
			minute = minutes % 60;
			hour = hours % 24;
			day = (days + 5) % 30;
			month = (days / 29 - 2) % 12;
			year = years + 1970;
			*/
		//	std::cout<<"\nCurrent time: "<<year<<"-"<<month<<"-"<<day<<" "<<hour<<":"<<minute<<":"<<second;
			break;
		}
		case TimeType::MILLISECONDS_NO_CALENDER:
		{
			millisecond = intervals % 1000;
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