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
	Init(currentTime);
}

/// Initialize with random seed.
void Random::Init(int64 seed)
{
	x = seed >> 8;
	y = seed >> 16;
	z = seed >> 24;
	w = seed >> 32;
}


/// Returns a random value between 0.0f and 1.0f
float Random::Randf(float max)
{ 
	unsigned int t;
	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ t ^ (t >> 8);
	// Divide by int-max
	float res = w / 4294967295.f;
	return res * max;
}

/// Returns a random value between 0 and max (inclusive)
int Random::Randi(int max)
{
	return Randf() * max;
}
