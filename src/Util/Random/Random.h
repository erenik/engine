/// Emil Hedemalm
/// 2014-06-14
/// Randomizer function/class.

#ifndef RANDOM_H

// A random number generator. base don the XOR shift
// http://en.wikipedia.org/wiki/Xorshift
class Random 
{
public:
	Random();
	/// Returns a random value between 0.0f and 1.0f
	float Randf();
private:
	unsigned int x, y, z, w;
};

#endif RANDOM_H

