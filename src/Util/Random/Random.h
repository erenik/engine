/// Emil Hedemalm
/// 2014-06-14
/// Randomizer function/class.

#ifndef RANDOM_H
#define RANDOM_H

#include "System/DataTypes.h"

// A random number generator. base don the XOR shift
// http://en.wikipedia.org/wiki/Xorshift
class Random 
{
public:
	Random();
	/// Initialize with random seed.
	void Init(int64 seed);
	/// Returns a random value between 0.0f and 1.0f
	float Randf(float max = 1.f);
	/// Returns a random value between 0 and max (inclusive)
	int Randi(int max);
private:
	unsigned int x, y, z, w;
};

#endif RANDOM_H
