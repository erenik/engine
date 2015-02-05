/// Emil Hedemalm
/// 2014-07-16
/// Time-class, made to handle time values based on some certain starting point in time.

#ifndef TIME_H
#define TIME_H

#include "String/AEString.h"
#include "System/DataTypes.h"

namespace TimeType
{
	enum {
		UNDEFINED,
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms724284%28v=vs.85%29.aspx
		WIN32_100NANOSEC_SINCE_JAN1_1601, 
	};
};

/// There can only be one time.
#undef Time
class Time 
{
public:
	/// 0 time.
	Time();
	/// Undefined time.
	Time(uint64 intervals);
	/// Time using a given type and starting-point. 
	Time(uint64 intervals, int type);

	/// Returns current time in default time-type/-format.
	static Time Now();

	/** Valid format, where the following characters will be replaced as follows:
		Y - year in 4 digits
		M - month in 2 digits
		D - day in 2 digits
		H - hours in 2 digits
		N - minute in 2 digits
		S - seconds in 2 digits

		Fetches calender data implicitly on each call.
	*/
	String ToString(String withFormat);

	/// Arithmetic operations
	Time operator - (const Time & otherTime) const;
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
	void ConvertTo(int type);

	// Current total in micro-seconds since the starting-point.
	int64 Microseconds();
	int64 Milliseconds();
	/// Total time in seconds.
	int64 Seconds();

	/// Calender/watch-type Accessors.
	int Second();
	int Minute();
	int Hour();
	int Day();
	int Week();
	int Month();
	int Year();

	/// The amount of intervals in this time. Exactly how intervals is used is up to which functions you wish to use.
	uint64 intervals;
	
	/// o-o
	int Type(){ return type;};
private:
	/// Fetches calender data given the intervals and type defined now.
	void FetchCalenderData();

	int second;
	int minute;
	int hour;
	int day;
	int month;
	int year;

	/// See enumeratoin above. Defines the duration of one interval, but may also affect how the accessors work (e.g. for other calender-types).
	int type;
};

#endif

