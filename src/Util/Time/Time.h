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
	Time(int intervals);
	/// Time using a given type and starting-point. 
	Time(int intervals, int type);

	/// Returns current time in default time-type/-format.
	static Time Now();

	/** Valid format, where the following characters will be replaced as follows:
		h - hours in 2 digits
		m - minute in 2 digits
		s - seconds in 2 digits

		Fetches calender data implicitly on each call.
	*/
	String ToString(String withFormat);

	/// Arithmetic operations
	Time operator - (const Time & otherTime) const;

	// Conversion operators
	operator const int64 () const;
	operator const int32 () const;
	

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
	int64 intervals;
	
	/// o-o
	int Type(){ return type;};
private:
	/// Fetches calender data given the intervals and type defined now.
	void FetchCalenderData();

	int second;
	int minute;
	int hour;
	int day;

	/// See enumeratoin above. Defines the duration of one interval, but may also affect how the accessors work (e.g. for other calender-types).
	int type;
};

#endif

