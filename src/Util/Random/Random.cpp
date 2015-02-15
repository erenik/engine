/// Emil Hedemalm
/// 2014-06-14
/// Randomizer function/class.

#include "Random.h"
#include "Timer/Timer.h"

#include <iostream>

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
	x = seed * 3;
	y = seed * 7;
	z = seed * 15;
	w = seed * 16;

	p = seed % 15 + 1;

	s[0] = seed;
	s[1] = 1181783497276652981;
	for (int i = 2; i < XOR_SHIFT_STAR_VALUES; ++i)
	{
		s[i] = i;
	}

//	std::cout<<"\nInitial values: ";
	for (int i = 0; i < XOR_SHIFT_STAR_VALUES; ++i)
	{
//		std::cout<<"\n"<<i<<": "<<s[i];
	}

	/// Randomize it a few times to get it started?
	for (int i = 0; i < XOR_SHIFT_STAR_VALUES * 3 + seed % 50; ++i)
	{
		Randf(1.f);
//		std::cout<<"\nRandf: "<<Randf(1.f);
	}
}


/// Returns a random value between 0.0f and 1.0f
float Random::Randf(float max)
{ 
 
	uint64 s0 = s[ p ];
	uint64 s1 = s[ p = ( p + 1 ) & XOR_SHIFT_STAR_VALUES - 1];
	s1 ^= s1 << 31; // a
	s1 ^= s1 >> 11; // b
	s0 ^= s0 >> 30; // c

	uint64 value = ( s[ p ] = s0 ^ s1 ) * 18446744073709551615;

	float res = value / (double) 18446744073709551615;
	return res * max;

	/*
	unsigned int t;
	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ t ^ (t >> 8);
	// Divide by int-max
	float res = w / 4294967295.f;
	return res * max;
	*/
}

/// Returns a random value between 0 and max (inclusive)
int Random::Randi(int max)
{
	return Randf(max);
}

/// Returns a random vlaue between 0 and max (inclusive)
int64 Random::Rand64(int64 max)
{
	int64 rand64 = Randi(INT_MAX);
	rand64 *= 0xFFFFFFFF;
	rand64 += Randi(INT_MAX);
	int64 modulated = rand64 % max;
	return modulated;
}