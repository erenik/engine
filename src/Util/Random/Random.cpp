/// Emil Hedemalm
/// 2014-06-14
/// Randomizer function/class.

#include "Random.h"
#include "Timer/Timer.h"

Random::Random()
{
	// Initialize the variables
	// Just use current time in micro-seconds somehow?
	int64 currentTime = Timer::GetCurrentTimeMicro();
	x = currentTime;
	y = currentTime >> 4;
	z = currentTime >> 8;
	w = currentTime >> 12;
}

/// Returns a random value between 0.0f and 1.0f
float Random::Randf()
{ 
	unsigned int t;
	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ t ^ (t >> 8);
	// Divide by int-max
	float res = w / 4294967295.f;
	return res;
}