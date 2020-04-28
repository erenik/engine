/// Emil Hedemalm
/// 2014-07-16
/// Time-class, made to handle time values based on some certain starting point in time.

#ifndef TIME_H
#define TIME_H

#include "String/AEString.h"
#include "System/DataTypes.h"

enum TimeType {
	UNDEFINED,
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724284%28v=vs.85%29.aspx
	WIN32_100NANOSEC_SINCE_JAN1_1601, 
	// Default linux time and gettimeofday function return value. See linux.die.net/man/2/time and linux.die.net/man/2/gettimeofday
	LINUX_MICROSEC_SINCE_JAN1_1970,
	// Custom type only used for handling in-game minute/hour/day cycles within e.g. the Weather system.
	MILLISECONDS_NO_CALENDER,
	TIME_TYPES
};

/// There can only be one time.
#define Time AETime
class AETime 
{
public:
	/// 0 time.
	Time();
	Time(TimeType type);
	/// Time using a given type and starting-point. 
	Time(TimeType type, uint64 intervals);
	/// Unit test.
	static void Test();
	/// o.o
	static Time Milliseconds(int amount);
	/// Returns current time in default time-type/-format.
	static Time Now();
	/// Dicates default type of created Time objects. Default is TimeType::UNDEFINED
	static TimeType defaultType;

	/** Valid format, where the following characters will be replaced as follows:
		Y - year in 4 digits
		M - month in 2 digits
		D - day in 2 digits
		H - hours in 2 digits
		m - minute in 2 digits
		S - seconds in 2 digits
		n - millisecond in 3 digits
		Fetches calender data implicitly on each call.
	*/
	String ToString(String withFormat);

	/// Arithmetic operations
	Time operator - (const Time & otherTime) const;
	Time operator + (const Time & otherTime) const;
	Time operator * (const float & f) const;
	/// Arithmetic operations
	Time operator -= (const Time & otherTime);
	Time operator += (const Time & otherTime);

	/// Comparison operators
	/// Larger than, returns true if this time is larger (more recent) than the other time. False if not.
	bool operator > (const Time & otherTime);
	/// Lesser than, returns true if this time is smaller (older) than the other time. False if not.
	bool operator < (const Time & otherTime);
	bool operator == (const Time & otherTime);

	// Conversion operators
	operator const int64 () const;
	operator const int32 () const;
	
	/// Converts this time into specified type.
	void ConvertTo(TimeType type);

	/// o.o
	void AddMs(int amount);
	/// Sets the hour in current day by adding or removing diff amount of intervals.
	void SetHour(int hour);
	/// Sets the hour in current hour by adding or removing diff amount of intervals.
	void SetMinute(int minute);
	void SetSecond(int second);
	void SetMillisecond(int newMilli);
	// Current total in micro-seconds since the starting-point.
	uint64 Microseconds() const;
	int64 Milliseconds() const;
	/// Total time in seconds.
	int64 Seconds() const;
	int64 Minutes() const;

	/// Calender/watch-type Accessors.
	int Millisecond(); // 0 to 999
	int Second(); // 0 to 59
	int Minute(); // 0 to 59
	int Hour(); // 0 to 23?
	int Day(); // 1 to 30? Depends on calender D:
	int Week();
	int Month();
	int Year();

	/// The amount of intervals in this time. Exactly how intervals is used is up to which functions you wish to use.
	uint64 intervals;

	/// File I/O
	bool WriteTo(std::fstream & stream);
	bool ReadFrom(std::fstream & stream);
	/// String I/O
	bool ParseFrom(const String & string);
	/// Prints type, intervals, and relevant unit
	void PrintData();
	/// o-o
	TimeType Type(){ return type;};

	int IntervalsPerHour();
	int IntervalsPerMinute();
	int IntervalsPerSecond();
	int IntervalsPerMillisecond();
private:
	/// Fetches calender data given the intervals and type defined now.
	virtual void FetchCalenderData();

	int millisecond;
	int second;
	int minute;
	int hour;
	int day;
	int month;
	int year;

	/// See enumeratoin above. Defines the duration of one interval, but may also affect how the accessors work (e.g. for other calender-types).
	TimeType type;
};

#endif

