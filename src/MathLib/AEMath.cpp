/// Emil Hedemalm
/// 2015-02-08 (much older originally)
/// Aeonic Games Math

#include "AEMath.h"
#include <cstdlib>

float oneDivRandMaxFloat = 0;

// Initializes the various numbers specified above.
void MathLib::Init()
{
	oneDivRandMaxFloat = 1.f / RAND_MAX;
}
